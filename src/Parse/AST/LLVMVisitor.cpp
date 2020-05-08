#include "scarpch.hpp"
#include "Parse/AST/LLVMVisitor.hpp"

#pragma warning(push, 0)
#pragma warning(disable:4996) // Deprecation
#pragma warning(disable:4146) // Operator minus on unsigned type
#pragma warning(disable:4244) // Type casts
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar.h>
#include "llvm/Transforms/Utils.h"
#pragma warning(pop)

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // MISCELANEOUS

        static llvm::Type* ASTTypeLLVMType(Type::VariableType type, llvm::LLVMContext& context) {
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

        static unsigned int ASTTypeBits(Type::VariableType type) {
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

        static bool ASTTypeSigned(Type::VariableType type) {
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
            void Add(std::string_view name, llvm::AllocaInst* value) { m_Symbols.back()[name] = value; }
            void PushScope() { m_Symbols.push_back({}); }
            void PopScope() { m_Symbols.pop_back(); }

            llvm::AllocaInst* Find(std::string_view name) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(name);
                    if (item != iter->end()) {
                        return item->second;
                    }
                }
                return nullptr;
            }

        private:
            std::vector<std::unordered_map<std::string_view, llvm::AllocaInst*>> m_Symbols = { {} };
        };

        struct LoopBlocks {
            llvm::BasicBlock* Header;
            llvm::BasicBlock* Exit;
            LoopBlocks(llvm::BasicBlock* header, llvm::BasicBlock* exit) : Header(header), Exit(exit) {}
        };

        struct VisitorData {
            SymbolTable Symbols;
            std::vector<LoopBlocks> LoopStack;
        };
        static VisitorData s_Data;

        LLVMVisitor::LLVMVisitor() :
            m_Builder(m_Context),
            m_Module(MakeScope<llvm::Module>("test_module", m_Context)),
            m_FunctionPassManager(MakeScope<llvm::legacy::FunctionPassManager>(m_Module.get()))
        {
            m_FunctionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            m_FunctionPassManager->add(llvm::createInstructionCombiningPass());
            m_FunctionPassManager->add(llvm::createReassociatePass());
            m_FunctionPassManager->add(llvm::createGVNPass());
            m_FunctionPassManager->add(llvm::createCFGSimplificationPass());
            m_FunctionPassManager->doInitialization();
        }

        void LLVMVisitor::ThrowError(std::string_view msg, const Span& span) {
            throw ParseError(ErrorCode::Unknown, [&]() {
                SCAR_ERROR("{}: {}", span, msg);
            });
        }

        static llvm::AllocaInst* CreateEntryAlloca(llvm::Function* func, llvm::Type* type, const std::string& name)  {
            llvm::IRBuilder<> builder = llvm::IRBuilder<>(&func->getEntryBlock(), func->getEntryBlock().begin());
            return builder.CreateAlloca(type, 0, name.c_str());
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void LLVMVisitor::Visit(Type& node) {
            switch (node.VarType) {
            case Type::Void:
                m_ReturnType = llvm::Type::getDoubleTy(m_Context);
                return;
            case Type::F64:
                m_ReturnType = llvm::Type::getDoubleTy(m_Context);
                return;
            default:
                SCAR_BUG("missing LLVM IR code for VariableType {}", node.VarType);
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
            llvm::Function* func = m_Module->getFunction(node.Prototype->Name.GetString().data());

            // Generate prototype if not already declared
            if (!func) {
                node.Prototype->Accept(*this);
                func = llvm::cast<llvm::Function>(m_ReturnValue);

                if (!func) {
                    m_ReturnValue = nullptr;
                    return;
                }
            }

            llvm::BasicBlock* block = llvm::BasicBlock::Create(m_Context, "entry", func);

            s_Data.Symbols.PushScope();
            m_Builder.SetInsertPoint(block);

            // Add argument names to symbol table
            for (auto& arg : func->args()) {
                llvm::AllocaInst* alloc = CreateEntryAlloca(func, arg.getType(), arg.getName());
                m_Builder.CreateStore(&arg, alloc);

                s_Data.Symbols.Add(arg.getName().data(), alloc);
            }

            node.CodeBlock->Accept(*this);
            m_BlockReturned = false;

            s_Data.Symbols.PopScope();

            // Make sure we have a return value
            if (!m_ReturnValue) {
                func->eraseFromParent();
                return;
            }

            // Verify function code
            if (llvm::verifyFunction(*func, &llvm::errs())) {
                func->eraseFromParent();
                m_ReturnValue = nullptr;
                return;
            }
            // Optimize function
            m_FunctionPassManager->run(*func);

            m_ReturnValue = func;
        }

        void LLVMVisitor::Visit(FunctionPrototype& node) {
            // Argument types
            std::vector<llvm::Type*> argTypes;
            argTypes.reserve(node.Args.size());
            for (auto& arg : node.Args) {
                arg.VarType->Accept(*this);
                argTypes.push_back(m_ReturnType);
            }
            // Return type
            node.ReturnType->Accept(*this);
            llvm::Type* retType = m_ReturnType;

            // Create function
            llvm::FunctionType* funcType = llvm::FunctionType::get(retType, argTypes, false);
            llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.Name.GetString().data(), *m_Module);

            // Set argument names
            unsigned int index = 0;
            for (auto& arg : func->args()) {
                arg.setName(node.Args[index].Name.GetString().data());
            }

            m_ReturnValue = func;
            return;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void LLVMVisitor::Visit(Branch& node) {
            llvm::Function* func = m_Builder.GetInsertBlock()->getParent();

            llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(m_Context, "branch.true", func);
            llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(m_Context, "branch.false", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(m_Context, "branch.exit");

            s_Data.Symbols.PushScope();

            node.Condition->Accept(*this);
            llvm::Value* cond = m_Builder.CreateFCmpONE(m_ReturnValue, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "cond");
            m_Builder.CreateCondBr(cond, trueBlock, falseBlock);

            bool needExit = false;

            // True block
            m_Builder.SetInsertPoint(trueBlock);
            node.TrueBlock->Accept(*this);
            llvm::Value* trueRet = m_ReturnValue;
            if (!m_BlockReturned) {
                m_Builder.CreateBr(exitBlock);
                needExit |= true;
            }
            m_BlockReturned = false;
            trueBlock = m_Builder.GetInsertBlock();

            s_Data.Symbols.PopScope();
            s_Data.Symbols.PushScope();

            // False block
            m_Builder.SetInsertPoint(falseBlock);
            node.FalseBlock->Accept(*this);
            llvm::Value* falseRet = m_ReturnValue;
            if (!m_BlockReturned) {
                m_Builder.CreateBr(exitBlock);
                needExit |= true;
            }
            m_BlockReturned = false;
            falseBlock = m_Builder.GetInsertBlock();

            // Exit
            s_Data.Symbols.PopScope();

            if (needExit) {
                func->getBasicBlockList().push_back(exitBlock);
                m_Builder.SetInsertPoint(exitBlock);

                // Merge branch return value
                /*llvm::PHINode* phi = m_Builder.CreatePHI(llvm::Type::getDoubleTy(m_Context), (unsigned int)mergers.size(), "merge");
                for (auto& merger : mergers) {
                    phi->addIncoming(merger.first, merger.second);
                }
                m_ReturnValue = phi;*/
            }
            else {
                m_BlockReturned = true;
            }
        }

        void LLVMVisitor::Visit(ForLoop& node) {
            llvm::Function* func = m_Builder.GetInsertBlock()->getParent();

            llvm::BasicBlock* headerBlock = llvm::BasicBlock::Create(m_Context, "loop.header", func);
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(m_Context, "loop.body", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(m_Context, "loop.exit", func);
            s_Data.LoopStack.push_back({ headerBlock, exitBlock });

            s_Data.Symbols.PushScope();

            node.Init->Accept(*this);
            m_Builder.CreateBr(headerBlock);

            // Header block
            m_Builder.SetInsertPoint(headerBlock);
            node.Condition->Accept(*this);
            llvm::Value* cond = m_Builder.CreateFCmpONE(m_ReturnValue, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "cond");
            m_Builder.CreateCondBr(cond, bodyBlock, exitBlock);

            // Body block
            m_Builder.SetInsertPoint(bodyBlock);
            node.CodeBlock->Accept(*this);
            if (!m_BlockReturned) {
                node.Update->Accept(*this);
                m_Builder.CreateBr(headerBlock);
            }
            m_BlockReturned = false;

            // Exit
            s_Data.Symbols.PopScope();
            s_Data.LoopStack.pop_back();
            m_Builder.SetInsertPoint(exitBlock);
        }

        void LLVMVisitor::Visit(WhileLoop& node) {
            llvm::Function* func = m_Builder.GetInsertBlock()->getParent();

            llvm::BasicBlock* headerBlock = llvm::BasicBlock::Create(m_Context, "loop.header", func);
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(m_Context, "loop.body", func);
            llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(m_Context, "loop.exit", func);
            s_Data.LoopStack.push_back({ headerBlock, exitBlock });

            s_Data.Symbols.PushScope();
            m_Builder.CreateBr(headerBlock);

            // Header block
            m_Builder.SetInsertPoint(headerBlock);
            node.Condition->Accept(*this);
            llvm::Value* cond = m_Builder.CreateFCmpONE(m_ReturnValue, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "cond");
            m_Builder.CreateCondBr(cond, bodyBlock, exitBlock);

            // Body block
            m_Builder.SetInsertPoint(bodyBlock);
            node.CodeBlock->Accept(*this);
            if (!m_BlockReturned) {
                m_Builder.CreateBr(headerBlock);
            }
            m_BlockReturned = false;

            // Exit
            s_Data.Symbols.PopScope();
            s_Data.LoopStack.pop_back();
            m_Builder.SetInsertPoint(exitBlock);
        }

        void LLVMVisitor::Visit(Block& node) {
            for (auto& item : node.Items) {
                item->Accept(*this);
                if (m_BlockReturned) {
                    return;
                }
            }
        }

        void LLVMVisitor::Visit(Continue& node) {
            m_Builder.CreateBr(s_Data.LoopStack.back().Header);
            m_BlockReturned = true;
        }

        void LLVMVisitor::Visit(Break& node) {
            m_Builder.CreateBr(s_Data.LoopStack.back().Exit);
            m_BlockReturned = true;
        }

        void LLVMVisitor::Visit(Return& node) {
            node.Value->Accept(*this);
            m_Builder.CreateRet(m_ReturnValue);
            m_BlockReturned = true;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void LLVMVisitor::Visit(FunctionCall& node) {
            llvm::Function* func = m_Module->getFunction(node.Name.GetString().data());
            if (!func) {
                ThrowError(FMT("undeclared funcation call: {}", node.Name.GetString()), node.GetSpan());
                return;
            }

            if (func->arg_size() != node.Args.size()) {
                ThrowError(FMT("incorrect number of arguments: {}", node.Name.GetString()), node.GetSpan());
                return;
            }

            std::vector<llvm::Value*> argValues;
            for (size_t i = 0; i < node.Args.size(); i++) {
                node.Args[i]->Accept(*this);
                argValues.push_back(m_ReturnValue);
                if (!m_ReturnValue) {
                    return;
                }
            }

            m_ReturnValue = m_Builder.CreateCall(func, argValues, "call");
        }

        void LLVMVisitor::Visit(Var& node) {
            llvm::Function* func = m_Builder.GetInsertBlock()->getParent();

            node.VarType->Accept(*this);
            llvm::AllocaInst* alloc = CreateEntryAlloca(func, m_ReturnType, node.Name.GetString().data());
            s_Data.Symbols.Add(node.Name.GetString(), alloc);
            node.Assign->Accept(*this);
        }

        void LLVMVisitor::Visit(Variable& node) {
            m_ReturnValue = s_Data.Symbols.Find(node.Name.GetString());
            if (!m_ReturnValue) {
                ThrowError(FMT("undeclared variable: {}", node.Name.GetString()), node.GetSpan());
            }
            m_ReturnValue = m_Builder.CreateLoad(m_ReturnValue, node.Name.GetString().data());
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
                m_ReturnValue = m_Builder.CreateFNeg(m_ReturnValue, "fneg");
                break;
            case PrefixOperator::Not:
                node.RHS->Accept(*this);
                m_ReturnValue = m_Builder.CreateFCmpUNE(m_ReturnValue, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "fcmpone");
                m_ReturnValue = m_Builder.CreateNot(m_ReturnValue, "not");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case PrefixOperator::BitNot:
                node.RHS->Accept(*this);
                m_ReturnValue = m_Builder.CreateNot(m_ReturnValue, "bitnot");
                break;

            default:
                SCAR_BUG("missing LLVM IR code for prefix operator {}", (int)node.Type);
                m_ReturnValue = nullptr;
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
                m_ReturnValue = nullptr;
                break;
            }
            node.LHS->Accept(*this);
        }

        void LLVMVisitor::Visit(BinaryOperator& node) {

            if (node.Type == BinaryOperator::Assign) {
                // Make sure LHS is a variable
                Variable* lhsNode = dynamic_cast<Variable*>(node.LHS.get());
                if (!lhsNode) {
                    ThrowError("expected a variable", node.LHS->GetSpan());
                }
                node.RHS->Accept(*this);
                llvm::Value* val = m_ReturnValue;
                
                // Get variable alloca
                llvm::Value* var = s_Data.Symbols.Find(lhsNode->Name.GetString());
                if (!var) {
                    ThrowError("undeclared variable", node.RHS->GetSpan());
                }

                m_Builder.CreateStore(val, var);
                m_ReturnValue = val;
                return;
            }

            node.LHS->Accept(*this);
            llvm::Value* lhs = m_ReturnValue;
            node.RHS->Accept(*this);
            llvm::Value* rhs = m_ReturnValue;

            if (!lhs || !rhs) {
                m_ReturnValue = nullptr;
                return;
            }

            switch (node.Type) {
            case BinaryOperator::MemberAccess:
                SCAR_BUG("BinaryOperator::MemberAccess not implemented");
                break;
            case BinaryOperator::Multiply:
                m_ReturnValue = m_Builder.CreateFMul(lhs, rhs, "fmul");
                break;
            case BinaryOperator::Divide:
                m_ReturnValue = m_Builder.CreateFDiv(lhs, rhs, "fdiv");
                break;
            case BinaryOperator::Reminder:
                m_ReturnValue = m_Builder.CreateFRem(lhs, rhs, "frem");
                break;
            case BinaryOperator::Plus:
                m_ReturnValue = m_Builder.CreateFAdd(lhs, rhs, "fadd");
                break;
            case BinaryOperator::Minus:
                m_ReturnValue = m_Builder.CreateFSub(lhs, rhs, "fsub");
                break;
            case BinaryOperator::Greater:
                m_ReturnValue = m_Builder.CreateFCmpUGT(lhs, rhs, "fcmpugt");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::GreaterEq:
                m_ReturnValue = m_Builder.CreateFCmpUGE(lhs, rhs, "fcmpuge");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::Lesser:
                m_ReturnValue = m_Builder.CreateFCmpULT(lhs, rhs, "fcmpult");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::LesserEq:
                m_ReturnValue = m_Builder.CreateFCmpULE(lhs, rhs, "fcmpule");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::Eq:
                m_ReturnValue = m_Builder.CreateFCmpUEQ(lhs, rhs, "fcmpueq");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::NotEq:
                m_ReturnValue = m_Builder.CreateFCmpUNE(lhs, rhs, "fcmpune");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::BitAnd:
                m_ReturnValue = m_Builder.CreateAnd(lhs, rhs, "and");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::BitXOr:
                m_ReturnValue = m_Builder.CreateXor(lhs, rhs, "xor");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::BitOr:
                m_ReturnValue = m_Builder.CreateOr(lhs, rhs, "or");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::LogicAnd:
                lhs = m_Builder.CreateFCmpONE(lhs, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "lhscmp");
                rhs = m_Builder.CreateFCmpONE(rhs, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "rhscmp");
                m_ReturnValue = m_Builder.CreateAnd(lhs, rhs, "and");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
            case BinaryOperator::LogicOr:
                lhs = m_Builder.CreateFCmpONE(lhs, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "lhscmp");
                rhs = m_Builder.CreateFCmpONE(rhs, llvm::ConstantFP::get(m_Context, llvm::APFloat(0.0)), "rhscmp");
                m_ReturnValue = m_Builder.CreateOr(lhs, rhs, "or");
                m_ReturnValue = m_Builder.CreateUIToFP(m_ReturnValue, llvm::Type::getDoubleTy(m_Context), "fbool");
                break;
          
            default:
                SCAR_BUG("missing LLVM IR code for binary operator {}", node.Type);
                m_ReturnValue = nullptr;
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void LLVMVisitor::Visit(LiteralBool& node) {
            llvm::Type* type = ASTTypeLLVMType(node.GetValueType(), m_Context);
            unsigned int bits = ASTTypeBits(node.GetValueType());
            m_ReturnValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value));
        }

        void LLVMVisitor::Visit(LiteralInteger& node) {
            llvm::Type* type = ASTTypeLLVMType(node.GetValueType(), m_Context);
            unsigned int bits = ASTTypeBits(node.GetValueType());
            bool sign = ASTTypeSigned(node.GetValueType());
            m_ReturnValue = llvm::ConstantInt::get(type, llvm::APInt(bits, node.Value, sign));
        }

        void LLVMVisitor::Visit(LiteralFloat& node) {
            llvm::Type* type = ASTTypeLLVMType(node.GetValueType(), m_Context);
            m_ReturnValue = llvm::ConstantFP::get(type, llvm::APFloat(node.Value));
        }

        void LLVMVisitor::Visit(LiteralString& node) {
            m_ReturnValue = nullptr;
        }

    }
}