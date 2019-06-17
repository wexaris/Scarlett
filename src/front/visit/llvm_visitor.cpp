#include "llvm_visitor.hpp"
#include "front/interner.hpp"
#include "log/logging.hpp"

namespace scar {
    namespace ast {

        LLVMVisitor::LLVMVisitor(const shared<SymbolStack>& symbols) :
            builder(context),
            symbol_stack(symbols),
            module("test", context)
        {}

#define RETURN_(x)  return x
#define RETURN      RETURN_(true)
#define RETURN_FAIL RETURN_(false)

#define RETURN_VALUE(x)      value = x; RETURN
#define RETURN_VALUE_FAIL(x) value = x; RETURN_FAIL
#define RETURN_TYPE(x)       type = x; RETURN
#define RETURN_TYPE_FAIL(x)  type = x; RETURN_FAIL


        bool LLVMVisitor::visit(ExprPreIncrement& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprPostIncrement& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprPostDecrement& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprPreDecrement& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprNegative& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprNot& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprBitNot& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprDeref& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprAddress& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(ExprMemberAccess& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(ExprSum& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateFAdd(lhs, rhs, "add_tmp"));
        }

        bool LLVMVisitor::visit(ExprSub& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateFSub(lhs, rhs, "sub_tmp"));
        }

        bool LLVMVisitor::visit(ExprMul& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateFMul(lhs, rhs, "mul_tmp"));
        }

        bool LLVMVisitor::visit(ExprDiv& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateFDiv(lhs, rhs, "div_tmp"));
        }

        bool LLVMVisitor::visit(ExprMod& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateFRem(lhs, rhs, "mod_tmp"));
        }

        bool LLVMVisitor::visit(ExprLogicAnd& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(nullptr);
            //builder.CreateAnd(lhs, rhs, "bitand_tmp");
        }

        bool LLVMVisitor::visit(ExprLogicOr& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(nullptr);
            //builder.CreateOr(lhs, rhs, "bitor_tmp");
        }

        bool LLVMVisitor::visit(ExprBitAnd& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateAnd(lhs, rhs, "bitand_tmp"));
        }

        bool LLVMVisitor::visit(ExprBitOr& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateOr(lhs, rhs, "bitor_tmp"));
        }

        bool LLVMVisitor::visit(ExprBitXOr& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateXor(lhs, rhs, "xor_tmp"));
        }

        bool LLVMVisitor::visit(ExprShl& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateShl(lhs, rhs, "shl_tmp"));
        }

        bool LLVMVisitor::visit(ExprShr& node) {
            node.lhs->accept(*this);
            auto lhs = value;
            node.rhs->accept(*this);
            auto rhs = value;

            if (!lhs || !rhs) {
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(builder.CreateLShr(lhs, rhs, "lshr_tmp"));
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(StrExpr& node) {
            RETURN_VALUE(llvm::ConstantDataArray::getString(context, node.val.get_strref(), false));
        }
        bool LLVMVisitor::visit(CharExpr& node) {
            RETURN_VALUE(llvm::ConstantDataArray::getString(context, node.val.get_strref(), false));
        }
        bool LLVMVisitor::visit(BoolExpr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(1, node.val, true))
        }

        bool LLVMVisitor::visit(I8Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(8, node.val, true))
        }
        bool LLVMVisitor::visit(I16Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(16, node.val, true))
        }
        bool LLVMVisitor::visit(I32Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(32, node.val, true))
        }
        bool LLVMVisitor::visit(I64Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(64, node.val, true))
        }

        bool LLVMVisitor::visit(U8Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(8, node.val))
        }
        bool LLVMVisitor::visit(U16Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(16, node.val))
        }
        bool LLVMVisitor::visit(U32Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(32, node.val))
        }
        bool LLVMVisitor::visit(U64Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<double>(node.val))));
            //llvm::ConstantInt::get(context, llvm::APInt(64, node.val))
        }

        bool LLVMVisitor::visit(F32Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(node.val)));
        }
        bool LLVMVisitor::visit(F64Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(node.val)));
        }

        bool LLVMVisitor::visit(VarExpr& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(FunCall& node) {
            auto fun = module.getFunction(node.path.back().get_strref()); // FIXME: support full paths
            if (!fun) {
                log::error("function not found");
                RETURN_VALUE_FAIL(nullptr);
            }

            std::vector<llvm::Value*> args;
            args.reserve(node.args.size());

            for (auto& arg : node.args) {
                arg->accept(*this);
                if (!value) {
                    RETURN_VALUE_FAIL(nullptr);
                }
                args.push_back(value);
            }

            RETURN_VALUE(builder.CreateCall(fun, args, "call_tmp"));
        }

        bool LLVMVisitor::visit(Block& node) {
            bool success = true;
            for (auto& stmt : node.stmts) {
                success &= stmt->accept(*this);
            }
            RETURN_(success);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(VarDecl& node) {
            (void)node;
            RETURN_VALUE(nullptr);
        }

        bool LLVMVisitor::visit(FunPrototypeDecl& node) {
            std::vector<llvm::Type*> param_types;
            param_types.resize(node.params.size(), llvm::Type::getDoubleTy(context));
            
            auto fun_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), param_types, false);

            auto fun = llvm::Function::Create(fun_type, llvm::Function::CommonLinkage, node.name.get_strref(), module);
        
            unsigned i = 0;
            for (auto& arg : fun->args()) {
                arg.setName(node.params[i++].name.get_strref());
            }

            RETURN_VALUE(fun);
        }

        bool LLVMVisitor::visit(FunDecl& node) {
            // Check if the function's been forward declared
            auto fun = module.getFunction(node.prototype->name.get_strref());

            // Attempt to visit the function's prototype
            if (!fun) {
                node.prototype->accept(*this);
                fun = llvm::dyn_cast<llvm::Function>(value);
                if (!fun) {
                    RETURN_VALUE_FAIL(nullptr);
                }
            }

            // Check if the function has a body
            // Return if it doesn't
            if (!node.block) {
                RETURN_VALUE(fun);
            }

            // Check for body redefinition
            if (!fun->empty()) {
                log::error("function body can't be redefined");
                RETURN_VALUE_FAIL(nullptr);
            }

            // Create a new block
            auto block = llvm::BasicBlock::Create(context, "entry", fun);
            builder.SetInsertPoint(block);

            // Push a new table to the symbol stack
            // Record the function's parameter names
            symbol_stack->push();
            for (auto& arg : fun->args()) {
                auto name = Interner::instance().find(arg.getName()).value();
                auto sym = sym::Symbol(name, sym::Var);
                symbol_stack->top()->insert(sym, sym::SymInfo(node.prototype.get()));
            }

            // Visit the function's block
            // Erase it if it's invalid
            if (!node.block->accept(*this)) {
                block->eraseFromParent();
                RETURN_VALUE_FAIL(nullptr);
            }
            builder.CreateRet(value);

            if (!llvm::verifyFunction(*fun)) {
                block->eraseFromParent();
                log::critical("function generation failed");
                RETURN_VALUE_FAIL(nullptr);
            }

            symbol_stack->pop();
            RETURN_VALUE(fun);
        }

        bool LLVMVisitor::visit(ExprStmt& node) {
            RETURN_(node.expr->accept(*this));
        }

        bool LLVMVisitor::visit(Module& node) {
            bool success = true;
            for (auto& stmt : node.stmts) {
                success &= stmt->accept(*this);
            }
            RETURN_(success);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(StrType&) {
            RETURN_TYPE(llvm::ArrayType::getInt32Ty(context));
        }

        bool LLVMVisitor::visit(CharType&) {
            RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
        }

        bool LLVMVisitor::visit(BoolType&) {
            RETURN_TYPE(llvm::IntegerType::getInt1Ty(context));
        }

        bool LLVMVisitor::visit(I8Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt8Ty(context));
        }

        bool LLVMVisitor::visit(I16Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt16Ty(context));
        }

        bool LLVMVisitor::visit(I32Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
        }

        bool LLVMVisitor::visit(I64Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt64Ty(context));
        }

        bool LLVMVisitor::visit(U8Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt8Ty(context));
        }

        bool LLVMVisitor::visit(U16Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt16Ty(context));
        }

        bool LLVMVisitor::visit(U32Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
        }

        bool LLVMVisitor::visit(U64Type&) {
            RETURN_TYPE(llvm::IntegerType::getInt64Ty(context));
        }

        bool LLVMVisitor::visit(F32Type&) {
            RETURN_TYPE(llvm::Type::getFloatTy(context));
        }

        bool LLVMVisitor::visit(F64Type&) {
            RETURN_TYPE(llvm::Type::getDoubleTy(context));
        }

        bool LLVMVisitor::visit(CustomType& node) {
            (void)node;
            RETURN_TYPE(nullptr);
        }

    }
}