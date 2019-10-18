#pragma once

namespace scar {
    namespace ast {

        class ASTVisitor {

        public:
            ASTVisitor() = default;
            virtual ~ASTVisitor() = default;

            virtual bool visit(struct Type& node) = 0;

			virtual bool visit(struct BinOp& node) = 0;
			virtual bool visit(struct UnaryOp& node) = 0;

            virtual bool visit(struct StrExpr& node) = 0;
            virtual bool visit(struct CharExpr& node) = 0;
            virtual bool visit(struct BoolExpr& node) = 0;
            virtual bool visit(struct I8Expr& node) = 0;
            virtual bool visit(struct I16Expr& node) = 0;
            virtual bool visit(struct I32Expr& node) = 0;
            virtual bool visit(struct I64Expr& node) = 0;
            virtual bool visit(struct U8Expr& node) = 0;
            virtual bool visit(struct U16Expr& node) = 0;
            virtual bool visit(struct U32Expr& node) = 0;
            virtual bool visit(struct U64Expr& node) = 0;
            virtual bool visit(struct F32Expr& node) = 0;
            virtual bool visit(struct F64Expr& node) = 0;
            virtual bool visit(struct VarExpr& node) = 0;
            virtual bool visit(struct FunCall& node) = 0;
            virtual bool visit(struct Block& node) = 0;

            virtual bool visit(struct VarDecl& node) = 0;
            virtual bool visit(struct FunPrototypeDecl& node) = 0;
            virtual bool visit(struct FunDecl& node) = 0;
			virtual bool visit(struct RetVoidStmt& node) = 0;
			virtual bool visit(struct RetStmt& node) = 0;
			virtual bool visit(struct ExprStmt& node) = 0;
            virtual bool visit(struct Module& node) = 0;
            
        };

    }
}