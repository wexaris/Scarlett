#pragma once

namespace scar {
    namespace ast {

        class ASTVisitor {

        public:
            ASTVisitor() = default;
            virtual ~ASTVisitor() = default;

            virtual bool visit(struct ExprPreIncrement& node) = 0;
            virtual bool visit(struct ExprPreDecrement& node) = 0;
            virtual bool visit(struct ExprPostIncrement& node) = 0;
            virtual bool visit(struct ExprPostDecrement& node) = 0;
            virtual bool visit(struct ExprNegative& node) = 0;
            virtual bool visit(struct ExprNot& node) = 0;
            virtual bool visit(struct ExprBitNot& node) = 0;
            virtual bool visit(struct ExprDeref& node) = 0;
            virtual bool visit(struct ExprAddress& node) = 0;
            
            virtual bool visit(struct ExprMemberAccess& node) = 0;
            virtual bool visit(struct ExprSum& node) = 0;
            virtual bool visit(struct ExprSub& node) = 0;
            virtual bool visit(struct ExprMul& node) = 0;
            virtual bool visit(struct ExprDiv& node) = 0;
            virtual bool visit(struct ExprMod& node) = 0;
            virtual bool visit(struct ExprLogicAnd& node) = 0;
            virtual bool visit(struct ExprLogicOr& node) = 0;
            virtual bool visit(struct ExprBitAnd& node) = 0;
            virtual bool visit(struct ExprBitOr& node) = 0;
            virtual bool visit(struct ExprBitXOr& node) = 0;
            virtual bool visit(struct ExprShl& node) = 0;
            virtual bool visit(struct ExprShr& node) = 0;

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
            virtual bool visit(struct ExprStmt& node) = 0;
            virtual bool visit(struct Module& node) = 0;
            
            virtual bool visit(struct StrType& node) = 0;
            virtual bool visit(struct CharType& node) = 0;
            virtual bool visit(struct BoolType& node) = 0;
            virtual bool visit(struct I8Type& node) = 0;
            virtual bool visit(struct I16Type& node) = 0;
            virtual bool visit(struct I32Type& node) = 0;
            virtual bool visit(struct I64Type& node) = 0;
            virtual bool visit(struct U8Type& node) = 0;
            virtual bool visit(struct U16Type& node) = 0;
            virtual bool visit(struct U32Type& node) = 0;
            virtual bool visit(struct U64Type& node) = 0;
            virtual bool visit(struct F32Type& node) = 0;
            virtual bool visit(struct F64Type& node) = 0;
            virtual bool visit(struct CustomType& node) = 0;
        };

    }
}