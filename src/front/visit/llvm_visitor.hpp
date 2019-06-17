#pragma once
#include "visitor.hpp"
#include "front/sym_table.hpp"
#include "llvm.hpp"

namespace scar {
    namespace ast {

        class LLVMVisitor : public ASTVisitor {

        private:
            llvm::LLVMContext context;
            llvm::IRBuilder<> builder;
            shared<SymbolStack> symbol_stack;

            llvm::Value* value = nullptr;
            llvm::Type* type = nullptr;

        public:
            llvm::Module module;

            LLVMVisitor(const shared<SymbolStack>& symbols);

            bool visit(ExprPreIncrement& node)  override;
            bool visit(ExprPreDecrement& node)  override;
            bool visit(ExprPostIncrement& node)  override;
            bool visit(ExprPostDecrement& node)  override;
            bool visit(ExprNegative& node)  override;
            bool visit(ExprNot& node)  override;
            bool visit(ExprBitNot& node)  override;
            bool visit(ExprDeref& node)  override;
            bool visit(ExprAddress& node)  override;

            bool visit(ExprMemberAccess& node)  override;
            bool visit(ExprSum& node)  override;
            bool visit(ExprSub& node)  override;
            bool visit(ExprMul& node)  override;
            bool visit(ExprDiv& node)  override;
            bool visit(ExprMod& node)  override;
            bool visit(ExprLogicAnd& node)  override;
            bool visit(ExprLogicOr& node)  override;
            bool visit(ExprBitAnd& node)  override;
            bool visit(ExprBitOr& node)  override;
            bool visit(ExprBitXOr& node)  override;
            bool visit(ExprShl& node)  override;
            bool visit(ExprShr& node)  override;

            bool visit(StrExpr& node)  override;
            bool visit(CharExpr& node)  override;
            bool visit(BoolExpr& node)  override;
            bool visit(I8Expr& node)  override;
            bool visit(I16Expr& node)  override;
            bool visit(I32Expr& node)  override;
            bool visit(I64Expr& node)  override;
            bool visit(U8Expr& node)  override;
            bool visit(U16Expr& node)  override;
            bool visit(U32Expr& node)  override;
            bool visit(U64Expr& node)  override;
            bool visit(F32Expr& node)  override;
            bool visit(F64Expr& node)  override;
            bool visit(VarExpr& node)  override;
            bool visit(FunCall& node)  override;
            bool visit(Block& node)  override;

            bool visit(VarDecl& node)  override;
            bool visit(FunPrototypeDecl& node)  override;
            bool visit(FunDecl& node)  override;
            bool visit(ExprStmt& node)  override;
            bool visit(Module& node)  override;

            bool visit(StrType& node)  override;
            bool visit(CharType& node)  override;
            bool visit(BoolType& node)  override;
            bool visit(I8Type& node)  override;
            bool visit(I16Type& node)  override;
            bool visit(I32Type& node)  override;
            bool visit(I64Type& node)  override;
            bool visit(U8Type& node)  override;
            bool visit(U16Type& node)  override;
            bool visit(U32Type& node)  override;
            bool visit(U64Type& node)  override;
            bool visit(F32Type& node)  override;
            bool visit(F64Type& node)  override;
            bool visit(CustomType& node)  override;
        };

    }
}