#pragma once
#include "visitor.hpp"
#include "ast/llvm.hpp"

namespace scar {
	namespace ast {

		class LLVMOptimizeVisitor : public ASTVisitor {

		public:
			std::shared_ptr<llvm::Module> module;

			LLVMOptimizeVisitor(std::shared_ptr<llvm::Module>& module);

			bool visit(BinOp& node) override;
			bool visit(UnaryOp& node) override;

			bool visit(StrExpr& node) override;
			bool visit(CharExpr& node) override;
			bool visit(BoolExpr& node) override;
			bool visit(I8Expr& node) override;
			bool visit(I16Expr& node) override;
			bool visit(I32Expr& node) override;
			bool visit(I64Expr& node) override;
			bool visit(U8Expr& node) override;
			bool visit(U16Expr& node) override;
			bool visit(U32Expr& node) override;
			bool visit(U64Expr& node) override;
			bool visit(F32Expr& node) override;
			bool visit(F64Expr& node) override;
			bool visit(VarExpr& node) override;
			bool visit(FunCall& node) override;
			bool visit(Block& node) override;

			bool visit(VarDecl& node) override;
			bool visit(FunPrototypeDecl& node) override;
			bool visit(FunDecl& node) override;
			bool visit(RetVoidStmt& node) override;
			bool visit(RetStmt& node) override;
			bool visit(ExprStmt& node) override;
			bool visit(Module& node) override;

			bool visit(Type& node) override;
		};

	}
}