#include "scarpch.hpp"
#include "Parse/AST/LLVMVisitor.hpp"
#include "Parse/AST/SymbolTable.hpp"

#ifdef _MSC_VER
    #pragma warning(push, 0)
    #pragma warning(disable:4996) // Deprecation
    #pragma warning(disable:4146) // Operator minus on unsigned type
    #pragma warning(disable:4244) // Type casts
#endif
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar.h>
#include "llvm/Transforms/Utils.h"
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#define SPAN_ERROR(msg, span) SCAR_ERROR("{}: {}", span, msg)

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        struct LLVMVisitorSymbol {
            llvm::AllocaInst* Alloca;
            llvm::Type* Type;
            llvm::StringRef Name;
        };
        class LLVMVisitorSymbolTable : public SymbolTable<std::string, LLVMVisitorSymbol> {
        public:
            LLVMVisitorSymbolTable() = default;
            ~LLVMVisitorSymbolTable() = default;

            void Add(const Ident& key, llvm::AllocaInst* val) { Add(key.GetString(), val); }
            void Add(const std::string& key, llvm::AllocaInst* val) {
                m_Symbols.back()[key] = LLVMVisitorSymbol{ val, val->getType(), val->getName() };
            }

            const LLVMVisitorSymbol& Find(const Ident& key) const { return Find(key.GetString()); }
            const LLVMVisitorSymbol& Find(const std::string& key) const {
                auto ret = TryFind(key);
                if (!ret) SCAR_CRITICAL("Symbol '{}' not found in SymbolTable!", key);
                return *ret;
            }
        };

        struct LoopBlocks {
            llvm::BasicBlock* Header;
            llvm::BasicBlock* Exit;
            LoopBlocks(llvm::BasicBlock* header, llvm::BasicBlock* exit) : Header(header), Exit(exit) {}
        };

        struct LLVMVisitorData {
            llvm::LLVMContext Context;
            Scope<llvm::IRBuilder<>> Builder;
            Scope<llvm::legacy::FunctionPassManager> FunctionPassManager;
            Scope<llvm::Module> Module;

            LLVMVisitorSymbolTable Symbols;
            std::vector<LoopBlocks> LoopStack;
            bool BlockReturned = false;

            // Return values across codegen functions
            llvm::AllocaInst* RetAlloca = nullptr;
            llvm::Value* RetValue = nullptr;
            llvm::Type* RetType = nullptr;
        };
        static LLVMVisitorData s_Data;

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // SUPPORT

        static llvm::Type* LLVMType(TypeInfo type) {
            switch (type) {
            case scar::ast::TypeInfo::Void: return llvm::Type::getVoidTy(s_Data.Context);

            case scar::ast::TypeInfo::Bool: return llvm::Type::getInt1Ty(s_Data.Context);

            case scar::ast::TypeInfo::I8:  return llvm::Type::getInt8Ty(s_Data.Context);
            case scar::ast::TypeInfo::I16: return llvm::Type::getInt16Ty(s_Data.Context);
            case scar::ast::TypeInfo::I32: return llvm::Type::getInt32Ty(s_Data.Context);
            case scar::ast::TypeInfo::I64: return llvm::Type::getInt64Ty(s_Data.Context);

            case scar::ast::TypeInfo::U8:  return llvm::Type::getInt8Ty(s_Data.Context);
            case scar::ast::TypeInfo::U16: return llvm::Type::getInt16Ty(s_Data.Context);
            case scar::ast::TypeInfo::U32: return llvm::Type::getInt32Ty(s_Data.Context);
            case scar::ast::TypeInfo::U64: return llvm::Type::getInt64Ty(s_Data.Context);

            case scar::ast::TypeInfo::F32: return llvm::Type::getFloatTy(s_Data.Context);
            case scar::ast::TypeInfo::F64: return llvm::Type::getDoubleTy(s_Data.Context);

            case scar::ast::TypeInfo::Char:
                SCAR_BUG("missing llvm::Type for Type::Char");
                break;
            case scar::ast::TypeInfo::String:
                SCAR_BUG("missing llvm::Type for Type::String");
                break;
            default:
                SCAR_BUG("missing llvm::Type for Type {}", (int)type);
                break;
            }
            return nullptr;
        }

        static unsigned int TypeBits(TypeInfo type) {
            switch (type) {
            case scar::ast::TypeInfo::Bool: return 1;

            case scar::ast::TypeInfo::I8:  return 8;
            case scar::ast::TypeInfo::I16: return 16;
            case scar::ast::TypeInfo::I32: return 32;
            case scar::ast::TypeInfo::I64: return 64;

            case scar::ast::TypeInfo::U8:  return 8;
            case scar::ast::TypeInfo::U16: return 16;
            case scar::ast::TypeInfo::U32: return 32;
            case scar::ast::TypeInfo::U64: return 64;

            case scar::ast::TypeInfo::F32: return 32;
            case scar::ast::TypeInfo::F64: return 64;

            case scar::ast::TypeInfo::Char:
                SCAR_BUG("missing bit count for Type::Char");
                break;
            default:
                SCAR_BUG("missing bit count for Type {}", (int)type);
                break;
            }
            return 0;
        }

        static bool TypeIsSigned(TypeInfo type) {
            switch (type) {
            case scar::ast::TypeInfo::I8:  return true;
            case scar::ast::TypeInfo::I16: return true;
            case scar::ast::TypeInfo::I32: return true;
            case scar::ast::TypeInfo::I64: return true;

            case scar::ast::TypeInfo::U8:  return false;
            case scar::ast::TypeInfo::U16: return false;
            case scar::ast::TypeInfo::U32: return false;
            case scar::ast::TypeInfo::U64: return false;

            case scar::ast::TypeInfo::Char:
                SCAR_BUG("missing bit count for Type::Char");
                break;
            default:
                SCAR_BUG("missing bit count for Type {}", (int)type);
                break;
            }
            return false;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        LLVMVisitor::LLVMVisitor() {
            s_Data.Module = MakeScope<llvm::Module>("test_module", s_Data.Context);
            s_Data.Builder = MakeScope<llvm::IRBuilder<>>(s_Data.Context);

            s_Data.FunctionPassManager = MakeScope<llvm::legacy::FunctionPassManager>(s_Data.Module.get());
            s_Data.FunctionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            s_Data.FunctionPassManager->add(llvm::createInstructionCombiningPass());
            s_Data.FunctionPassManager->add(llvm::createReassociatePass());
            s_Data.FunctionPassManager->add(llvm::createGVNPass());
            s_Data.FunctionPassManager->add(llvm::createCFGSimplificationPass());
            s_Data.FunctionPassManager->doInitialization();
        }

        void LLVMVisitor::Print() const {
            s_Data.Module->print(llvm::outs(), nullptr);
        }

        static llvm::AllocaInst* CreateEntryAlloca(llvm::Function* func, llvm::Type* type, const std::string& name)  {
            llvm::IRBuilder<> builder = llvm::IRBuilder<>(&func->getEntryBlock(), func->getEntryBlock().begin());
            return s_Data.Builder->CreateAlloca(type, 0, name);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void LLVMVisitor::Visit(Type& node) {
            switch (node.ResultType) {
            case TypeInfo::Void:
                s_Data.RetType = llvm::Type::getDoubleTy(s_Data.Context);
                return;
            case TypeInfo::I32:
                s_Data.RetType = llvm::Type::getInt32Ty(s_Data.Context);
                return;
            case TypeInfo::F64:
                s_Data.RetType = llvm::Type::getDoubleTy(s_Data.Context);
                return;
            default:
                SCAR_BUG("missing LLVM IR code for VariableType {}", node.ResultType);
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARATIONS

        void LLVMVisitor::Visit(Module& node) {
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
        }

        void LLVMVisitor::Visit(Function& node) {
            llvm::Function* func = s_Data.Module->getFunction(node.Prototype->Name.GetString());

            // Generate prototype if not already declared
            if (!func) {
                node.Prototype->Accept(*this);
                func = llvm::cast<llvm::Function>(s_Data.RetValue);

                if (!func) {
                    s_Data.RetValue = nullptr;
                    return;
                }
            }

            llvm::BasicBlock* block = llvm::BasicBlock::Create(s_Data.Context, "entry", func);

            s_Data.Symbols.PushScope();
            s_Data.Builder->SetInsertPoint(block);

            // Add argument names to symbol table
            for (auto& arg : func->args()) {
                llvm::AllocaInst* alloc = CreateEntryAlloca(func, arg.getType(), arg.getName());
                s_Data.Builder->CreateStore(&arg, alloc);
                s_Data.Symbols.Add(arg.getName(), alloc);
            }

            node.CodeBlock->Accept(*this);
            s_Data.BlockReturned = false;

            s_Data.Symbols.PopScope();

            // Make sure we have a return value
            if (!s_Data.RetValue) {
                func->eraseFromParent();
                return;
            }

            // Verify function code
            if (llvm::verifyFunction(*func, &llvm::errs())) {
                func->eraseFromParent();
                s_Data.RetValue = nullptr;
                return;
            }
            // Optimize function
            s_Data.FunctionPassManager->run(*func);

            s_Data.RetValue = func;
        }

        void LLVMVisitor::Visit(FunctionPrototype& node) {
            // Argument types
            std::vector<llvm::Type*> argTypes;
            argTypes.reserve(node.Args.size());
            for (auto& arg : node.Args) {
                arg.VarType->Accept(*this);
                argTypes.push_back(s_Data.RetType);
            }
            // Return type
            node.ReturnType->Accept(*this);
            llvm::Type* retType = s_Data.RetType;

            // Create function
            llvm::FunctionType* funcType = llvm::FunctionType::get(retType, argTypes, false);
            llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.Name.GetString(), *s_Data.Module);

            // Set argument names
            unsigned int index = 0;
            for (auto& arg : func->args()) {
                arg.setName(node.Args[index].Name.GetString());
            }

            s_Data.RetValue = func;
            return;
        }

        void LLVMVisitor::Visit(VarDecl& node) {
            llvm::Function* func = s_Data.Builder->GetInsertBlock()->getParent();

            node.VarType->Accept(*this);
            llvm::AllocaInst* alloc = CreateEntryAlloca(func, s_Data.RetType, node.Name.GetString());
            s_Data.Symbols.Add(node.Name, alloc);

            s_Data.RetAlloca = alloc;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void LLVMVisitor::Visit(Branch& node) {
            llvm::Function* func = s_Data.Builder->GetInsertBlock()->getParent();

            llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(s_Data.Context, "branch.true", func);
            llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(s_Data.Context, "branch.false", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(s_Data.Context, "branch.exit");

            s_Data.Symbols.PushScope();

            node.Condition->Accept(*this);
            llvm::Value* cond = s_Data.Builder->CreateFCmpONE(s_Data.RetValue, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "cond");
            s_Data.Builder->CreateCondBr(cond, trueBlock, falseBlock);

            bool needExit = false;

            // True block
            s_Data.Builder->SetInsertPoint(trueBlock);
            node.TrueBlock->Accept(*this);
            if (!s_Data.BlockReturned) {
                s_Data.Builder->CreateBr(exitBlock);
                needExit |= true;
            }
            s_Data.BlockReturned = false;
            trueBlock = s_Data.Builder->GetInsertBlock();

            s_Data.Symbols.PopScope();
            s_Data.Symbols.PushScope();

            // False block
            s_Data.Builder->SetInsertPoint(falseBlock);
            node.FalseBlock->Accept(*this);
            if (!s_Data.BlockReturned) {
                s_Data.Builder->CreateBr(exitBlock);
                needExit |= true;
            }
            s_Data.BlockReturned = false;
            falseBlock = s_Data.Builder->GetInsertBlock();

            // Exit
            s_Data.Symbols.PopScope();

            if (needExit) {
                func->getBasicBlockList().push_back(exitBlock);
                s_Data.Builder->SetInsertPoint(exitBlock);

                // Merge branch return value
                /*llvm::PHINode* phi = s_Data.Builder->CreatePHI(llvm::Type::getDoubleTy(s_Data.Context), (unsigned int)mergers.size(), "merge");
                for (auto& merger : mergers) {
                    phi->addIncoming(merger.first, merger.second);
                }
                s_Data.ReturnValue = phi;*/
            }
            else {
                s_Data.BlockReturned = true;
            }
        }

        void LLVMVisitor::Visit(ForLoop& node) {
            llvm::Function* func = s_Data.Builder->GetInsertBlock()->getParent();

            llvm::BasicBlock* headerBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.header", func);
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.body", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.exit", func);
            s_Data.LoopStack.push_back({ headerBlock, exitBlock });

            s_Data.Symbols.PushScope();

            node.Init->Accept(*this);
            s_Data.Builder->CreateBr(headerBlock);

            // Header block
            s_Data.Builder->SetInsertPoint(headerBlock);
            node.Condition->Accept(*this);
            llvm::Value* cond = s_Data.Builder->CreateFCmpONE(s_Data.RetValue, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "cond");
            s_Data.Builder->CreateCondBr(cond, bodyBlock, exitBlock);

            // Body block
            s_Data.Builder->SetInsertPoint(bodyBlock);
            node.CodeBlock->Accept(*this);
            if (!s_Data.BlockReturned) {
                node.Update->Accept(*this);
                s_Data.Builder->CreateBr(headerBlock);
            }
            s_Data.BlockReturned = false;

            // Exit
            s_Data.Symbols.PopScope();
            s_Data.LoopStack.pop_back();
            s_Data.Builder->SetInsertPoint(exitBlock);
        }

        void LLVMVisitor::Visit(WhileLoop& node) {
            llvm::Function* func = s_Data.Builder->GetInsertBlock()->getParent();

            llvm::BasicBlock* headerBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.header", func);
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.body", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(s_Data.Context, "loop.exit", func);
            s_Data.LoopStack.push_back({ headerBlock, exitBlock });

            s_Data.Symbols.PushScope();
            s_Data.Builder->CreateBr(headerBlock);

            // Header block
            s_Data.Builder->SetInsertPoint(headerBlock);
            node.Condition->Accept(*this);
            llvm::Value* cond = s_Data.Builder->CreateFCmpONE(s_Data.RetValue, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "cond");
            s_Data.Builder->CreateCondBr(cond, bodyBlock, exitBlock);

            // Body block
            s_Data.Builder->SetInsertPoint(bodyBlock);
            node.CodeBlock->Accept(*this);
            if (!s_Data.BlockReturned) {
                s_Data.Builder->CreateBr(headerBlock);
            }
            s_Data.BlockReturned = false;

            // Exit
            s_Data.Symbols.PopScope();
            s_Data.LoopStack.pop_back();
            s_Data.Builder->SetInsertPoint(exitBlock);
        }

        void LLVMVisitor::Visit(Block& node) {
            for (auto& item : node.Items) {
                item->Accept(*this);
                if (s_Data.BlockReturned) {
                    return;
                }
            }
        }

        void LLVMVisitor::Visit(Continue& node) {
            s_Data.Builder->CreateBr(s_Data.LoopStack.back().Header);
            s_Data.BlockReturned = true;
        }

        void LLVMVisitor::Visit(Break& node) {
            s_Data.Builder->CreateBr(s_Data.LoopStack.back().Exit);
            s_Data.BlockReturned = true;
        }

        void LLVMVisitor::Visit(Return& node) {
            node.Value->Accept(*this);
            s_Data.Builder->CreateRet(s_Data.RetValue);
            s_Data.BlockReturned = true;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void LLVMVisitor::Visit(FunctionCall& node) {
            llvm::Function* func = s_Data.Module->getFunction(node.Name.GetString());
            if (!func) {
                SPAN_ERROR(FMT("undeclared funcation call: {}", node.Name), node.GetSpan());
                return;
            }

            if (func->arg_size() != node.Args.size()) {
                SPAN_ERROR(FMT("incorrect number of arguments: {}", node.Name), node.GetSpan());
                return;
            }

            std::vector<llvm::Value*> argValues;
            for (size_t i = 0; i < node.Args.size(); i++) {
                node.Args[i]->Accept(*this);
                argValues.push_back(s_Data.RetValue);
                if (!s_Data.RetValue) {
                    return;
                }
            }

            s_Data.RetValue = s_Data.Builder->CreateCall(func, argValues, "call");
        }

        void LLVMVisitor::Visit(VarAccess& node) {
            s_Data.RetAlloca = s_Data.Symbols.Find(node.Name).Alloca;
            s_Data.RetValue = s_Data.Builder->CreateLoad(s_Data.RetAlloca, node.Name.GetString());
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void LLVMVisitor::Visit(PrefixOperator& node) {
            switch (node.Type) {
            case PrefixOperator::Increment:
                SCAR_BUG("PrefixOperator::Increment not implemented");
                break;
            case PrefixOperator::Decrement:
                SCAR_BUG("PrefixOperator::Decrement not implemented");
                break;
            case PrefixOperator::Plus:
                node.RHS->Accept(*this);
                break;
            case PrefixOperator::Minus:
                node.RHS->Accept(*this);
                s_Data.RetValue = s_Data.Builder->CreateFNeg(s_Data.RetValue, "fneg");
                break;
            case PrefixOperator::Not:
                node.RHS->Accept(*this);
                s_Data.RetValue = s_Data.Builder->CreateFCmpUNE(s_Data.RetValue, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "fcmpone");
                s_Data.RetValue = s_Data.Builder->CreateNot(s_Data.RetValue, "not");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "fbool");
                break;
            case PrefixOperator::BitNot:
                node.RHS->Accept(*this);
                s_Data.RetValue = s_Data.Builder->CreateNot(s_Data.RetValue, "bitnot");
                break;

            default:
                SCAR_BUG("missing LLVM IR code for prefix operator {}", (int)node.Type);
                s_Data.RetValue = nullptr;
                break;
            }
        }

        void LLVMVisitor::Visit(SuffixOperator& node) {
            node.LHS->Accept(*this);
            llvm::Value* lhs = s_Data.RetValue;

            switch (node.Type) {
            case SuffixOperator::Increment:
                SCAR_BUG("SuffixOperator::Increment not implemented");
                break;
            case SuffixOperator::Decrement:
                SCAR_BUG("SuffixOperator::Decrement not implemented");
                break;
            case SuffixOperator::Cast:
            {
                // Cast to bool
                if (node.ResultType.IsBool()) {
                    if (node.LHS->ResultType.IsInt()) {
                        // Compare lhs to (i32) zero
                        s_Data.RetValue = s_Data.Builder->CreateICmpNE(lhs, llvm::ConstantInt::get(llvm::Type::getInt32Ty(s_Data.Context), llvm::APInt(32, 0)), "cast");
                    }
                    else if (node.LHS->ResultType.IsFloat()) {
                        // Compare lhs to (f32) zero
                        s_Data.RetValue = s_Data.Builder->CreateFCmpUNE(lhs, llvm::ConstantFP::get(llvm::Type::getFloatTy(s_Data.Context), llvm::APFloat(0.0f)), "cast");
                    }
                }
                // Cast to sint or uint
                if (node.ResultType.IsInt()) {
                    if (node.LHS->ResultType.IsBool() || node.LHS->ResultType.IsInt()) {
                        // Basic int cast
                        s_Data.RetValue = s_Data.Builder->CreateIntCast(lhs, LLVMType(node.ResultType), node.ResultType.IsSInt(), "cast");
                    }
                    else if (node.LHS->ResultType.IsFloat()) {
                        // Convert from floating to sint/uint
                        if (node.ResultType.IsSInt())
                            s_Data.RetValue = s_Data.Builder->CreateFPToSI(lhs, LLVMType(node.ResultType), "cast");
                        else
                            s_Data.RetValue = s_Data.Builder->CreateFPToUI(lhs, LLVMType(node.ResultType), "cast");
                    }
                }
                // Cast to float
                else if (node.ResultType.IsFloat()) {
                    if (node.LHS->ResultType.IsBool() || node.LHS->ResultType.IsInt()) {
                        // Convert from sint/uint to floating point
                        if (node.LHS->ResultType.IsUInt())
                            s_Data.RetValue = s_Data.Builder->CreateUIToFP(lhs, LLVMType(node.ResultType), "cast");
                        else
                            s_Data.RetValue = s_Data.Builder->CreateSIToFP(lhs, LLVMType(node.ResultType), "cast");
                    }
                    else if (node.LHS->ResultType.IsFloat()) {
                        // Basic floating point cast
                        s_Data.RetValue = s_Data.Builder->CreateFPCast(lhs, LLVMType(node.ResultType), "cast");
                    }
                }
                // Cast to char
                else if (node.ResultType.IsChar()) {
                    SCAR_BUG("missing LLVM IR code for cast to char");
                }
                // Cast to string
                else if (node.ResultType.IsString()) {
                    SCAR_BUG("missing LLVM IR code for cast to string");
                }
                else {
                    SPAN_ERROR("invalid cast", node.GetSpan());
                }
                break;
            }

            default:
                SCAR_BUG("missing LLVM IR code for prefix operator {}", node.Type);
                s_Data.RetValue = nullptr;
                break;
            }
        }

        void MakeBothFloat(llvm::Value*& lhs, llvm::Value*& rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isIntegerTy()) {
                if (TypeIsSigned(lhsNode->ResultType))
                    lhs = s_Data.Builder->CreateSIToFP(lhs, rhs->getType(), "sitofp");
                else lhs = s_Data.Builder->CreateUIToFP(lhs, rhs->getType(), "uitofp");
            }
            if (rhs->getType()->isIntegerTy()) {
                if (TypeIsSigned(rhsNode->ResultType))
                    rhs = s_Data.Builder->CreateSIToFP(rhs, lhs->getType(), "sitofp");
                else rhs = s_Data.Builder->CreateUIToFP(rhs, lhs->getType(), "uitofp");
            }
        }

        llvm::Value* CreateMul(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFMul(lhs, rhs, "fmul");
            }
            return s_Data.Builder->CreateMul(lhs, rhs, "mul");
        }

        llvm::Value* CreateDiv(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFDiv(lhs, rhs, "fdiv");
            }
            if (TypeIsSigned(lhsNode->ResultType) || TypeIsSigned(lhsNode->ResultType)) {
                return s_Data.Builder->CreateSDiv(lhs, rhs, "sdiv");
            }
            return s_Data.Builder->CreateUDiv(lhs, rhs, "udiv");
        }

        llvm::Value* CreateRem(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFRem(lhs, rhs, "frem");
            }
            if (TypeIsSigned(lhsNode->ResultType) || TypeIsSigned(lhsNode->ResultType)) {
                return s_Data.Builder->CreateSRem(lhs, rhs, "srem");
            }
            return s_Data.Builder->CreateURem(lhs, rhs, "urem");
        }

        llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFAdd(lhs, rhs, "fadd");
            }
            return s_Data.Builder->CreateAdd(lhs, rhs, "add");
        }

        llvm::Value* CreateSub(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFSub(lhs, rhs, "fsub");
            }
            return s_Data.Builder->CreateSub(lhs, rhs, "sub");
        }

        void LLVMVisitor::Visit(BinaryOperator& node) {

            switch (node.Type) {
            case BinaryOperator::Assign: {
                // Visit LHS
                node.LHS->Accept(*this);
                llvm::AllocaInst* alloc = s_Data.RetAlloca;

                // Visit RHS
                node.RHS->Accept(*this);
                llvm::Value* val = s_Data.RetValue;

                s_Data.Builder->CreateStore(val, alloc);
                s_Data.RetValue = val;
                return;
            }
            default: break;
            }

            node.LHS->Accept(*this);
            llvm::Value* lhs = s_Data.RetValue;
            node.RHS->Accept(*this);
            llvm::Value* rhs = s_Data.RetValue;

            if (!lhs || !rhs) {
                s_Data.RetValue = nullptr;
                return;
            }

            switch (node.Type) {
            case BinaryOperator::MemberAccess:
                SCAR_BUG("BinaryOperator::MemberAccess not implemented");
                break;
            case BinaryOperator::Multiply:
                s_Data.RetValue = CreateMul(lhs, rhs, node.LHS, node.RHS);
                break;
            case BinaryOperator::Divide:
                s_Data.RetValue = CreateDiv(lhs, rhs, node.LHS, node.RHS);
                break;
            case BinaryOperator::Remainder:
                s_Data.RetValue = CreateRem(lhs, rhs, node.LHS, node.RHS);
                break;
            case BinaryOperator::Plus:
                s_Data.RetValue = CreateAdd(lhs, rhs, node.LHS, node.RHS);
                break;
            case BinaryOperator::Minus:
                s_Data.RetValue = CreateSub(lhs, rhs, node.LHS, node.RHS);
                break;
            case BinaryOperator::Greater:
                s_Data.RetValue = s_Data.Builder->CreateFCmpUGT(lhs, rhs, "fcmpugt");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::GreaterEq:
                s_Data.RetValue = s_Data.Builder->CreateFCmpUGE(lhs, rhs, "fcmpuge");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::Lesser:
                s_Data.RetValue = s_Data.Builder->CreateFCmpULT(lhs, rhs, "fcmpult");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::LesserEq:
                s_Data.RetValue = s_Data.Builder->CreateFCmpULE(lhs, rhs, "fcmpule");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::Eq:
                s_Data.RetValue = s_Data.Builder->CreateFCmpUEQ(lhs, rhs, "fcmpueq");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::NotEq:
                s_Data.RetValue = s_Data.Builder->CreateFCmpUNE(lhs, rhs, "fcmpune");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::BitAnd:
                s_Data.RetValue = s_Data.Builder->CreateAnd(lhs, rhs, "and");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::BitXOr:
                s_Data.RetValue = s_Data.Builder->CreateXor(lhs, rhs, "xor");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::BitOr:
                s_Data.RetValue = s_Data.Builder->CreateOr(lhs, rhs, "or");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::LogicAnd:
                lhs = s_Data.Builder->CreateFCmpONE(lhs, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "lhscmp");
                rhs = s_Data.Builder->CreateFCmpONE(rhs, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "rhscmp");
                s_Data.RetValue = s_Data.Builder->CreateAnd(lhs, rhs, "and");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;
            case BinaryOperator::LogicOr:
                lhs = s_Data.Builder->CreateFCmpONE(lhs, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "lhscmp");
                rhs = s_Data.Builder->CreateFCmpONE(rhs, llvm::ConstantFP::get(s_Data.Context, llvm::APFloat(0.0)), "rhscmp");
                s_Data.RetValue = s_Data.Builder->CreateOr(lhs, rhs, "or");
                s_Data.RetValue = s_Data.Builder->CreateUIToFP(s_Data.RetValue, llvm::Type::getDoubleTy(s_Data.Context), "uitofp");
                break;

            default:
                SCAR_BUG("missing LLVM IR code for binary operator {}", node.Type);
                s_Data.RetValue = nullptr;
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void LLVMVisitor::Visit(LiteralBool& node) {
            llvm::Type* type = LLVMType(node.ResultType);
            unsigned int bits = TypeBits(node.ResultType);
            s_Data.RetValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value));
        }

        void LLVMVisitor::Visit(LiteralInteger& node) {
            llvm::Type* type = LLVMType(node.ResultType);
            unsigned int bits = TypeBits(node.ResultType);
            bool sign = TypeIsSigned(node.ResultType);
            s_Data.RetValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value, sign));
        }

        void LLVMVisitor::Visit(LiteralFloat& node) {
            llvm::Type* type = LLVMType(node.ResultType);
            s_Data.RetValue = llvm::ConstantFP::get(type, llvm::APFloat(node.Value));
        }

        void LLVMVisitor::Visit(LiteralString& node) {
            s_Data.RetValue = nullptr;
        }

    }
}