#include "scarpch.hpp"
#include "Parse/AST/LLVMVisitor.hpp"

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
        // MISCELANEOUS

        static llvm::Type* LLVMType(Type::ValueType type, llvm::LLVMContext& context) {
            switch (type) {
            case scar::ast::Type::Void: return llvm::Type::getVoidTy(context);

            case scar::ast::Type::Bool: return llvm::Type::getInt1Ty(context);

            case scar::ast::Type::I8:  return llvm::Type::getInt8Ty(context);
            case scar::ast::Type::I16: return llvm::Type::getInt16Ty(context);
            case scar::ast::Type::I32: return llvm::Type::getInt32Ty(context);
            case scar::ast::Type::I64: return llvm::Type::getInt64Ty(context);

            case scar::ast::Type::U8:  return llvm::Type::getInt8Ty(context);
            case scar::ast::Type::U16: return llvm::Type::getInt16Ty(context);
            case scar::ast::Type::U32: return llvm::Type::getInt32Ty(context);
            case scar::ast::Type::U64: return llvm::Type::getInt64Ty(context);

            case scar::ast::Type::F32: return llvm::Type::getFloatTy(context);
            case scar::ast::Type::F64: return llvm::Type::getDoubleTy(context);

            case scar::ast::Type::Char:
                SCAR_BUG("missing llvm::Type for Type::Char");
                break;
            case scar::ast::Type::String:
                SCAR_BUG("missing llvm::Type for Type::String");
                break;
            default:
                SCAR_BUG("missing llvm::Type for Type {}", (int)type);
                break;
            }
            return nullptr;
        }

        static unsigned int TypeBits(Type::ValueType type) {
            switch (type) {
            case scar::ast::Type::Bool: return 1;

            case scar::ast::Type::I8:  return 8;
            case scar::ast::Type::I16: return 16;
            case scar::ast::Type::I32: return 32;
            case scar::ast::Type::I64: return 64;

            case scar::ast::Type::U8:  return 8;
            case scar::ast::Type::U16: return 16;
            case scar::ast::Type::U32: return 32;
            case scar::ast::Type::U64: return 64;

            case scar::ast::Type::F32: return 32;
            case scar::ast::Type::F64: return 64;

            case scar::ast::Type::Char:
                SCAR_BUG("missing bit count for Type::Char");
                break;
            default:
                SCAR_BUG("missing bit count for Type {}", (int)type);
                break;
            }
            return 0;
        }

        static bool TypeIsSigned(Type::ValueType type) {
            switch (type) {
            case scar::ast::Type::I8:  return true;
            case scar::ast::Type::I16: return true;
            case scar::ast::Type::I32: return true;
            case scar::ast::Type::I64: return true;

            case scar::ast::Type::U8:  return false;
            case scar::ast::Type::U16: return false;
            case scar::ast::Type::U32: return false;
            case scar::ast::Type::U64: return false;

            case scar::ast::Type::Char:
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

        class SymbolTable {
        public:
            struct Symbol {
                llvm::AllocaInst* Alloca;
                llvm::Type* Type;
                llvm::StringRef Name;
            };

            SymbolTable() {
                PushScope();
            }

            void Add(const std::string& name, llvm::AllocaInst* symbol) {
                m_Symbols.back()[name] = Symbol{ symbol, symbol->getType(), symbol->getName() };
            }

            void PushScope() { m_Symbols.push_back({}); }
            void PopScope() { m_Symbols.pop_back(); }

            const Symbol& Find(const std::string& name) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(name);
                    if (item != iter->end()) {
                        return item->second;
                    }
                }
                SCAR_CRITICAL("Failed to identify symbol '{}'", name);
            }

        private:
            std::vector<std::unordered_map<std::string, Symbol>> m_Symbols;
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

            SymbolTable Symbols;
            std::vector<LoopBlocks> LoopStack;
            bool BlockReturned = false;

            // Return values across codegen functions
            llvm::Value* RetValue = nullptr;
            llvm::Type* RetType = nullptr;
        };
        static LLVMVisitorData s_Data{};

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
            return s_Data.Builder->CreateAlloca(type, 0, name.c_str());
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void LLVMVisitor::Visit(Type& node) {
            switch (node.ValType) {
            case Type::Void:
                s_Data.RetType = llvm::Type::getDoubleTy(s_Data.Context);
                return;
            case Type::I32:
                s_Data.RetType = llvm::Type::getInt32Ty(s_Data.Context);
                return;
            case Type::F64:
                s_Data.RetType = llvm::Type::getDoubleTy(s_Data.Context);
                return;
            default:
                SCAR_BUG("missing LLVM IR code for VariableType {}", node.ValType);
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
            llvm::Function* func = s_Data.Module->getFunction(node.Prototype->Name.GetString().data());

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
                s_Data.Symbols.Add(arg.getName().data(), alloc);
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
            llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.Name.GetString().data(), *s_Data.Module);

            // Set argument names
            unsigned int index = 0;
            for (auto& arg : func->args()) {
                arg.setName(node.Args[index].Name.GetString().data());
            }

            s_Data.RetValue = func;
            return;
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
            llvm::Function* func = s_Data.Module->getFunction(node.Name.GetString().data());
            if (!func) {
                SPAN_ERROR(FMT("undeclared funcation call: {}", node.Name.GetString()), node.GetSpan());
                return;
            }

            if (func->arg_size() != node.Args.size()) {
                SPAN_ERROR(FMT("incorrect number of arguments: {}", node.Name.GetString()), node.GetSpan());
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

        void LLVMVisitor::Visit(Var& node) {
            llvm::Function* func = s_Data.Builder->GetInsertBlock()->getParent();

            node.VarType->Accept(*this);
            llvm::AllocaInst* alloc = CreateEntryAlloca(func, s_Data.RetType, node.Name.GetString().data());
            s_Data.Symbols.Add(node.Name.GetString(), alloc);
            node.Assign->Accept(*this);
        }

        void LLVMVisitor::Visit(Variable& node) {
            s_Data.RetValue = s_Data.Symbols.Find(node.Name.GetString()).Alloca;
            if (!s_Data.RetValue) {
                SPAN_ERROR(FMT("undeclared variable: {}", node.Name.GetString()), node.GetSpan());
            }
            s_Data.RetValue = s_Data.Builder->CreateLoad(s_Data.RetValue, node.Name.GetString().data());
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
            switch (node.Type) {
            case SuffixOperator::Increment:
                SCAR_BUG("SuffixOperator::Increment not implemented");
                break;
            case SuffixOperator::Decrement:
                SCAR_BUG("SuffixOperator::Decrement not implemented");
                break;

            default:
                SCAR_BUG("missing LLVM IR code for prefix operator {}", node.Type);
                s_Data.RetValue = nullptr;
                break;
            }
            node.LHS->Accept(*this);
        }

        void MakeBothFloat(llvm::Value*& lhs, llvm::Value*& rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isIntegerTy()) {
                if (TypeIsSigned(lhsNode->ValueType))
                    lhs = s_Data.Builder->CreateSIToFP(lhs, rhs->getType(), "sitofp");
                else lhs = s_Data.Builder->CreateUIToFP(lhs, rhs->getType(), "uitofp");
            }
            if (rhs->getType()->isIntegerTy()) {
                if (TypeIsSigned(rhsNode->ValueType))
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
            if (TypeIsSigned(lhsNode->ValueType) || TypeIsSigned(lhsNode->ValueType)) {
                return s_Data.Builder->CreateSDiv(lhs, rhs, "sdiv");
            }
            return s_Data.Builder->CreateUDiv(lhs, rhs, "udiv");
        }

        llvm::Value* CreateRem(llvm::Value* lhs, llvm::Value* rhs, const Ref<Expr>& lhsNode, const Ref<Expr>& rhsNode) {
            if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy()) {
                MakeBothFloat(lhs, rhs, lhsNode, rhsNode);
                return s_Data.Builder->CreateFRem(lhs, rhs, "frem");
            }
            if (TypeIsSigned(lhsNode->ValueType) || TypeIsSigned(lhsNode->ValueType)) {
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
                // Make sure LHS is a variable
                Variable* lhsNode = dynamic_cast<Variable*>(node.LHS.get());
                if (!lhsNode) {
                    SPAN_ERROR("expected a variable", node.LHS->GetSpan());
                }
                // Visit RHS
                node.RHS->Accept(*this);
                llvm::Value* val = s_Data.RetValue;

                // Get variable allocator
                llvm::Value* var = s_Data.Symbols.Find(lhsNode->Name.GetString()).Alloca;
                if (!var) {
                    SPAN_ERROR("undeclared variable", node.RHS->GetSpan());
                }

                s_Data.Builder->CreateStore(val, var);
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
            llvm::Type* type = LLVMType(node.ValueType, s_Data.Context);
            unsigned int bits = TypeBits(node.ValueType);
            s_Data.RetValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value));
        }

        void LLVMVisitor::Visit(LiteralInteger& node) {
            llvm::Type* type = LLVMType(node.ValueType, s_Data.Context);
            unsigned int bits = TypeBits(node.ValueType);
            bool sign = TypeIsSigned(node.ValueType);
            s_Data.RetValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value, sign));
        }

        void LLVMVisitor::Visit(LiteralFloat& node) {
            llvm::Type* type = LLVMType(node.ValueType, s_Data.Context);
            s_Data.RetValue = llvm::ConstantFP::get(type, llvm::APFloat(node.Value));
        }

        void LLVMVisitor::Visit(LiteralString& node) {
            s_Data.RetValue = nullptr;
        }

    }
}