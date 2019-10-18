#include "llvm_opt_visitor.hpp"
#include "ast/ast.hpp"

namespace scar {
	namespace ast {

		LLVMOptimizeVisitor::LLVMOptimizeVisitor(std::shared_ptr<llvm::Module>& module) :
			module(module)
		{}

		bool LLVMOptimizeVisitor::visit(UnaryOp& node) {
			return node.expr->accept(*this);
		}


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		bool LLVMOptimizeVisitor::visit(BinOp& node) {
			return node.lhs->accept(*this) && node.rhs->accept(*this);
		}


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		bool LLVMOptimizeVisitor::visit(StrExpr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(CharExpr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(BoolExpr& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(I8Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(I16Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(I32Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(I64Expr& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(U8Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(U16Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(U32Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(U64Expr& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(F32Expr& node) {
			return true;
		}
		bool LLVMOptimizeVisitor::visit(F64Expr& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(VarExpr& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(FunCall& node) {
			for (auto& n : node.args) {
				if (!n->accept(*this)) {
					return false;
				}
			}
			return true;
		}

		bool LLVMOptimizeVisitor::visit(Block& node) {
			for (auto& n : node.stmts) {
				if (!n->accept(*this)) {
					return false;
				}
			}
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		bool LLVMOptimizeVisitor::visit(VarDecl& node) {
			return node.type->accept(*this) && node.val->accept(*this);
		}

		bool LLVMOptimizeVisitor::visit(FunPrototypeDecl& node) {
			for (auto& n : node.params) {
				if (!n.type->accept(*this)) {
					return false;
				}
			}
			if (!node.ret->accept(*this)) {
				return false;
			}
			return true;
		}

		bool LLVMOptimizeVisitor::visit(FunDecl& node) {
			if (!node.prototype->accept(*this) || !node.block->accept(*this)) {
				return false;
			}

			llvm::FunctionAnalysisManager analysis_mgr;
			llvm::PassBuilder builder;

			builder.registerFunctionAnalyses(analysis_mgr);
			auto pass_mgr = builder.buildFunctionSimplificationPipeline(
				llvm::PassBuilder::OptimizationLevel::O2,
				llvm::PassBuilder::ThinLTOPhase::None
			);

			// Run LLVM module optimization passes
			auto func = module->getFunction(node.prototype->name.get_strref());
			pass_mgr.run(*func, analysis_mgr);

			return true;
		}

		bool LLVMOptimizeVisitor::visit(RetVoidStmt& node) {
			return true;
		}

		bool LLVMOptimizeVisitor::visit(RetStmt& node) {
			return node.expr->accept(*this);
		}

		bool LLVMOptimizeVisitor::visit(ExprStmt& node) {
			return node.expr->accept(*this);
		}

		bool LLVMOptimizeVisitor::visit(Module& node) {
			for (auto& n : node.stmts) {
				if (!n->accept(*this)) {
					return false;
				}
			}
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		bool LLVMOptimizeVisitor::visit(Type& node) {
			return true;
		}

	}
}