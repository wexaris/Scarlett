#include "llvm_visitor.hpp"
#include "symbols/interner.hpp"
#include "driver/session.hpp"
#include "util/error.hpp"

namespace scar {
    namespace ast {

        LLVMVisitor::LLVMVisitor(log::LogManager& logger) :
            builder(context),
            logger(logger),
            module(std::make_shared<llvm::Module>("test", context))
        {}

		llvm::Value* value = nullptr; // Value return variable
		llvm::Type* type = nullptr;   // Type return variable

		inline llvm::AllocaInst* create_alloc(llvm::Function* func, llvm::Type* type, llvm::Value* arr_size, Name name) {
			llvm::IRBuilder<> build(&func->getEntryBlock(), func->getEntryBlock().begin());
			return build.CreateAlloca(type, arr_size, name.get_strref());
		}

		inline llvm::AllocaInst* create_alloc(llvm::Function* func, llvm::Type* type, const Name name) {
			return create_alloc(func, type, 0, name);
		}

#define RETURN_VALUE(x) value = x; return true
#define RETURN_TYPE(x) type = x; return true
#define RETURN_VALUE_FAIL(x) value = x; return false
#define RETURN_TYPE_FAIL(x) type = x; return false

        bool LLVMVisitor::visit(UnaryOp& node) {
			if (!node.expr->accept(*this)) {
				RETURN_VALUE_FAIL(nullptr);
			}

			switch (node.kind)
			{
			case UnaryOp::GlobalAccess:
				break;
			case UnaryOp::PreIncrement:
				break;
			case UnaryOp::PreDecrement:
				break;
			case UnaryOp::PostIncrement:
				break;
			case UnaryOp::PostDecrement:
				break;
			case UnaryOp::Positive:
				RETURN_VALUE(value);
			case UnaryOp::Negative:
				RETURN_VALUE(builder.CreateFNeg(value, "neg"));
			case UnaryOp::Not:
				RETURN_VALUE(builder.CreateNot(value, "not"));
			case UnaryOp::BitNot:
				RETURN_VALUE(builder.CreateNot(value, "bit_not"));
			case UnaryOp::Deref:
				break;
			case UnaryOp::Address:
				break;

			default:
				break;
			}

			logger.bug(FMT("LLVMVisitor: unary operator #{} not handled", (int)node.kind));
			RETURN_VALUE_FAIL(nullptr);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(BinOp& node) {
			node.lhs->accept(*this); // Safe because values are checked afterwards
			auto lhs = value;
			node.rhs->accept(*this); // Safe because values are checked afterwards
			auto rhs = value;

			if (!lhs || !rhs) {
				RETURN_VALUE_FAIL(nullptr);
			}

			switch (node.kind)
			{
			case BinOp::Scope:
				break;
			case BinOp::MemberAccess:
				break;
			case BinOp::Sum:
				RETURN_VALUE(builder.CreateFAdd(lhs, rhs, "fsum"));
			case BinOp::Sub:
				RETURN_VALUE(builder.CreateFSub(lhs, rhs, "fsub"));
			case BinOp::Mul:
				RETURN_VALUE(builder.CreateFMul(lhs, rhs, "fmul"));
			case BinOp::Div:
				RETURN_VALUE(builder.CreateFDiv(lhs, rhs, "fdiv"));
			case BinOp::Mod:
				RETURN_VALUE(builder.CreateFRem(lhs, rhs, "frem"));
			case BinOp::Eq:
				lhs = builder.CreateFCmpOEQ(lhs, rhs, "eq");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::NotEq:
				lhs = builder.CreateFCmpONE(lhs, rhs, "neq");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::Greater:
				lhs = builder.CreateFCmpOGT(lhs, rhs, "gt");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::GreaterEq:
				lhs = builder.CreateFCmpOGE(lhs, rhs, "geq");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::Lesser:
				lhs = builder.CreateFCmpOLT(lhs, rhs, "lt");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::LesserEq:
				lhs = builder.CreateFCmpOLE(lhs, rhs, "leq");
				RETURN_VALUE(builder.CreateUIToFP(lhs, llvm::Type::getDoubleTy(context), "ui_to_fp"));
			case BinOp::LogicAnd:
				break;
			case BinOp::LogicOr:
				break;
			case BinOp::BitAnd:
				RETURN_VALUE(builder.CreateAnd(lhs, rhs, "bit_and"));
			case BinOp::BitOr:
				RETURN_VALUE(builder.CreateOr(lhs, rhs, "bit_or"));
			case BinOp::BitXOr:
				RETURN_VALUE(builder.CreateXor(lhs, rhs, "bit_xor"));
			case BinOp::Shl:
				RETURN_VALUE(builder.CreateShl(lhs, rhs, "shl"));
			case BinOp::Shr:
				RETURN_VALUE(builder.CreateAShr(lhs, rhs, "ashr"));
			case BinOp::Assign: {
				// Only allow the LHS to be a variable
				auto var_node = dynamic_cast<VarExpr*>(node.lhs.get());
				if (!var_node) {
					logger.error("left hand side must be a variable");
					RETURN_VALUE_FAIL(nullptr);
				}
				// Attempt to find corresponding VarInfo
				auto info = variables.find(var_node->name);
				if (!info.has_value()) {
					logger.error("undeclared variable");
					RETURN_VALUE_FAIL(nullptr);
				}
				builder.CreateStore(rhs, info->value);
				RETURN_VALUE(rhs);
			}
			case BinOp::PlusAssign:
				break;
			case BinOp::MinusAssign:
				break;
			case BinOp::MulAssign:
				break;
			case BinOp::DivAssign:
				break;
			case BinOp::ModAssign:
				break;
			case BinOp::AndAssign:
				break;
			case BinOp::OrAssign:
				break;
			case BinOp::XorAssign:
				break;
			case BinOp::ShlAssign:
				break;
			case BinOp::ShrAssign:
				break;

			default:
				break;
			}

			logger.bug(FMT("LLVMVisitor: binary operator #{} not handled", (int)node.kind));
			RETURN_VALUE_FAIL(nullptr);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(StrExpr& node) {
            return true;
        }
        bool LLVMVisitor::visit(CharExpr& node) {
            return true;
        }
        bool LLVMVisitor::visit(BoolExpr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(1, (uint64_t)node.val, false)));
        }

        bool LLVMVisitor::visit(I8Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(8, (uint64_t)node.val, true)));
        }
        bool LLVMVisitor::visit(I16Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(16, (uint64_t)node.val, true)));
        }
        bool LLVMVisitor::visit(I32Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(32, (uint64_t)node.val, true)));
        }
        bool LLVMVisitor::visit(I64Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(64, node.val, true)));
        }

        bool LLVMVisitor::visit(U8Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(8, (uint64_t)node.val, false)));
        }
        bool LLVMVisitor::visit(U16Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(16, (uint64_t)node.val, false)));
        }
        bool LLVMVisitor::visit(U32Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(32, (uint64_t)node.val, false)));
        }
        bool LLVMVisitor::visit(U64Expr& node) {
            RETURN_VALUE(llvm::ConstantInt::get(context, llvm::APInt(64, node.val, false)));
        }

        bool LLVMVisitor::visit(F32Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(node.val)));
        }
        bool LLVMVisitor::visit(F64Expr& node) {
            RETURN_VALUE(llvm::ConstantFP::get(context, llvm::APFloat(node.val)));
        }

        bool LLVMVisitor::visit(VarExpr& node) {
            // FIXME: variable existence should already be guaranteed by previous check-visitors
            if (auto var = variables.find(node.name)) {
				RETURN_VALUE(builder.CreateLoad(var->type, var->value, node.name.get_strref()));
            }
            logger.bug(FMT("variable `{}` not declared", node.name));
			RETURN_VALUE_FAIL(nullptr);
        }

        bool LLVMVisitor::visit(FunCall& node) {
            // FIXME: function existence should already be guaranteed by previous check-visitors
            auto func = module->getFunction(node.path.back().get_strref());
            if (!func) {
                logger.bug(FMT("function `{}` not declared", node.path.back()));
				RETURN_VALUE_FAIL(nullptr);
            }
            // Check if argument count matches decl
            if (func->arg_size() != node.args.size()) {
                logger.bug(FMT("function `{}` argument mismatch", node.path[0]));
            }

            // Collect argument values
            std::vector<llvm::Value*> args;
            for (auto& arg : node.args) {
				if (!arg->accept(*this)) {
					RETURN_VALUE_FAIL(nullptr);
				}
                args.push_back(value);
            }

            RETURN_VALUE(builder.CreateCall(func, args, "call"));
        }

        bool LLVMVisitor::visit(Block& node) {
            for (auto& stmt : node.stmts) {
				if (!stmt->accept(*this)) {
					return false;
				}
            }
			return true;
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(VarDecl& node) {

			node.type->accept(*this); // Safe because values are checked afterwards
			auto ty = type;

			node.val->accept(*this); // Safe because values are checked afterwards
			auto val = value;

			if (!ty || !val) {
				return false;
			}

			auto func = builder.GetInsertBlock()->getParent();
			auto alloc = create_alloc(func, ty, node.name);
			builder.CreateStore(val, alloc);

			variables.insert(node.name, VarInfo{ ty, alloc });
			return true;
        }

        bool LLVMVisitor::visit(FunPrototypeDecl& node) {
            // Get arg types
            std::vector<llvm::Type*> params(node.params.size());
            for (unsigned i = 0; i < params.size(); i++) {
				if (!node.params[i].type->accept(*this)) {
					RETURN_VALUE_FAIL(nullptr);
				}
                params[i] = type;
            }
            // Get return type
			if (!node.ret->accept(*this)) {
				RETURN_VALUE_FAIL(nullptr);
			}
            auto ret = type;
            // Create function type
            auto types = llvm::FunctionType::get(ret, params, false);

            // Create function
            auto func = llvm::Function::Create(types, llvm::Function::ExternalLinkage, node.name.get_strref(), *module);
            
            // Assign names to the parameters
            unsigned idx = 0;
			for (auto& param : func->args()) {
				param.setName(node.params[idx++].name.get_strref());
			}

            RETURN_VALUE(func);
        }

        bool LLVMVisitor::visit(FunDecl& node) {
            
            // Check if function has already been declared,
            // if not, visit the prototype
			auto func = module->getFunction(node.prototype->name.get_strref());
            if (!func) {
                // Generate function from prototype
				if (!node.prototype->accept(*this)) {
					RETURN_VALUE_FAIL(nullptr);
				}
                func = llvm::cast<llvm::Function>(value);
                if (!func) {
                    RETURN_VALUE_FAIL(nullptr);
                }
            }

            // Check if already defined
            if (!func->empty()) {
                logger.error(FMT("function `{}` connot be redefined", node.prototype->name));
                RETURN_VALUE_FAIL(nullptr);
            }

            // Create function block
            auto block = llvm::BasicBlock::Create(context, "entry", func);
            builder.SetInsertPoint(block);

			// Save variable info in case their names are shadowed
			variables.push();
			auto param = node.prototype->params.begin();
            for (auto& arg : func->args()) {
				auto alloc = create_alloc(func, arg.getType(), param->name);
				builder.CreateStore(&arg, alloc);

                variables.insert(param->name, VarInfo{ arg.getType(), alloc });
				param++;
            }

            // Visit block
			if (!node.block->accept(*this)) {
				RETURN_VALUE_FAIL(nullptr);
			}

			// Pop variable stack
			variables.pop();

			// Check if the block had errors
            if (!value) {
				func->removeFromParent();
                RETURN_VALUE_FAIL(nullptr);
            }

            // Make sure function generated correctly
            if (llvm::verifyFunction(*func, &llvm::errs())) {
                logger.bug(FMT("function `{}` failed verification", node.prototype->name));
                func->removeFromParent();
                RETURN_VALUE_FAIL(nullptr);
            }

            RETURN_VALUE(func);
        }

		bool LLVMVisitor::visit(RetVoidStmt& node) {
			RETURN_VALUE(builder.CreateRetVoid());
		}

		bool LLVMVisitor::visit(RetStmt& node) {
			if (!node.expr->accept(*this)) {
				RETURN_VALUE_FAIL(nullptr);
			}
			RETURN_VALUE(builder.CreateRet(value));
		}

        bool LLVMVisitor::visit(ExprStmt& node) {
			return node.expr->accept(*this);
        }

        bool LLVMVisitor::visit(Module& node) {
            for (auto& stmt : node.stmts) {
				if (!stmt->accept(*this)) {
					return false;
				}
            }
            return true;
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        bool LLVMVisitor::visit(Type& node) {
			switch (node.kind)
			{
			case Type::Unknown:
				logger.bug("LLVMVisitor: encountered an unknown type");
				RETURN_VALUE_FAIL(nullptr);

			case Type::StrType:
				break;
			case Type::CharType:
				RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
			case Type::BoolType:
				RETURN_TYPE(llvm::IntegerType::getInt1Ty(context));
			case Type::I8Type:
				RETURN_TYPE(llvm::IntegerType::getInt8Ty(context));
			case Type::I16Type:
				RETURN_TYPE(llvm::IntegerType::getInt16Ty(context));
			case Type::I32Type:
				RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
			case Type::I64Type:
				RETURN_TYPE(llvm::IntegerType::getInt64Ty(context));
			case Type::U8Type:
				RETURN_TYPE(llvm::IntegerType::getInt8Ty(context));
			case Type::U16Type:
				RETURN_TYPE(llvm::IntegerType::getInt16Ty(context));
			case Type::U32Type:
				RETURN_TYPE(llvm::IntegerType::getInt32Ty(context));
			case Type::U64Type:
				RETURN_TYPE(llvm::IntegerType::getInt64Ty(context));
			case Type::F32Type:
				RETURN_TYPE(llvm::Type::getFloatTy(context));
			case Type::F64Type:
				RETURN_TYPE(llvm::Type::getDoubleTy(context));
			case Type::Custom:
				break;

			case Type::Void:
				RETURN_TYPE(llvm::Type::getVoidTy(context));
				break;

			default:
				break;
			}

			logger.bug(FMT("LLVMVisitor: type #{} not handled", (int)node.kind));
			RETURN_VALUE_FAIL(nullptr);
        }

    }
}