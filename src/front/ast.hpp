#pragma once
#include "token_type.hpp"
#include "visit/visitor.hpp"
#include <vector>
#include <string>
#include <memory>

namespace scar {

    template<typename T> using unique = std::unique_ptr<T>;
    template<typename T> using shared = std::shared_ptr<T>;
    template<typename T> using weak = std::weak_ptr<T>;
    using interned_str_t = std::shared_ptr<std::string>;

    namespace ast {

        struct Node {
            virtual ~Node() = default;
            virtual bool accept(ASTVisitor& v) = 0;
        };
        struct Stmt : virtual public Node {};
        struct Expr : virtual public Node {};
        struct Type : public Node {};

#define SCAR_AST_ACCEPT_OVERRIDE \
    bool accept(ASTVisitor& v) override { \
        return v.visit(*this); \
    }

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct Name {
            size_t id;
            Name(size_t id = 0);
            interned_str_t get_str() const;
            std::string& get_strref() const;
            bool operator==(const Name& other) const;
            bool operator!=(const Name& other) const;
        };

        using Path = std::vector<Name>;


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct UnaryOp : public Expr {
            unique<Expr> atom;
            UnaryOp(unique<Expr> atom);
        };

#define SCAR_AST_UNARYOP_DECL(name, ...) \
    struct name : public UnaryOp { \
        name(unique<Expr> atom) : \
            UnaryOp(std::move(atom)) {} \
        SCAR_AST_ACCEPT_OVERRIDE; \
        __VA_ARGS__\
    }

        SCAR_AST_UNARYOP_DECL(ExprPreIncrement, );
        SCAR_AST_UNARYOP_DECL(ExprPreDecrement, );
        SCAR_AST_UNARYOP_DECL(ExprPostIncrement, );
        SCAR_AST_UNARYOP_DECL(ExprPostDecrement, );
        SCAR_AST_UNARYOP_DECL(ExprNegative, );
        SCAR_AST_UNARYOP_DECL(ExprNot, );
        SCAR_AST_UNARYOP_DECL(ExprBitNot, );
        SCAR_AST_UNARYOP_DECL(ExprDeref, );
        SCAR_AST_UNARYOP_DECL(ExprAddress, );

#undef SCAR_AST_UNARYOP_DECL


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct BinOp : public Expr {
            unique<Expr> lhs;
            unique<Expr> rhs;
            BinOp(unique<Expr> lhs, unique<Expr> rhs);
        };

#define SCAR_AST_BINOP_DECL(name, ...) \
    struct name : public BinOp { \
        name(unique<Expr> lhs, unique<Expr> rhs) : \
            BinOp(std::move(lhs), std::move(rhs)) {} \
        SCAR_AST_ACCEPT_OVERRIDE; \
        __VA_ARGS__\
    }

        SCAR_AST_BINOP_DECL(ExprMemberAccess, );
        SCAR_AST_BINOP_DECL(ExprSum, );
        SCAR_AST_BINOP_DECL(ExprSub, );
        SCAR_AST_BINOP_DECL(ExprMul, );
        SCAR_AST_BINOP_DECL(ExprDiv, );
        SCAR_AST_BINOP_DECL(ExprMod, );
        SCAR_AST_BINOP_DECL(ExprLogicAnd, );
        SCAR_AST_BINOP_DECL(ExprLogicOr, );
        SCAR_AST_BINOP_DECL(ExprBitAnd, );
        SCAR_AST_BINOP_DECL(ExprBitOr, );
        SCAR_AST_BINOP_DECL(ExprBitXOr, );
        SCAR_AST_BINOP_DECL(ExprShl, );
        SCAR_AST_BINOP_DECL(ExprShr, );

#undef SCAR_AST_BINOP_DECL


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct StrExpr : public Expr {
            Name val;
            StrExpr(Name val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct CharExpr : public Expr {
            Name val;
            CharExpr(Name val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct BoolExpr : public Expr {
            int8_t val;
            BoolExpr(int8_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct I8Expr : public Expr {
            int8_t val;
            I8Expr(int8_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct I16Expr : public Expr {
            int16_t val;
            I16Expr(int16_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct I32Expr : public Expr {
            int32_t val;
            I32Expr(int32_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct I64Expr : public Expr {
            int64_t val;
            I64Expr(int64_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct U8Expr : public Expr {
            uint8_t val;
            U8Expr(int8_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct U16Expr : public Expr {
            uint16_t val;
            U16Expr(int16_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct U32Expr : public Expr {
            uint32_t val;
            U32Expr(int32_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct U64Expr : public Expr {
            uint64_t val;
            U64Expr(int64_t val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct F32Expr : public Expr {
            double val;
            F32Expr(double val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct F64Expr : public Expr {
            double val;
            F64Expr(double val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct VarExpr : public Expr {
            Name name;
            VarExpr(Name name);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        using ArgList = std::vector<unique<ast::Expr>>;

        struct FunCall : public Expr, public Stmt {
            Path path;
            ArgList args;
            FunCall(Path path, ArgList args);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Block : public Expr, public Stmt {
            using item_t = std::vector<std::unique_ptr<Stmt>>;
            item_t stmts;
            Block(item_t stmts);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct VarDecl : public Stmt {
            Name name;
            unique<Type> type;
            unique<Expr> val;
            VarDecl(Name name, unique<Type> ty, unique<Expr> val);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Param {
            Name name;
            unique<Type> type;
            Param(Name name, unique<Type> ty);
        };

        using ParamList = std::vector<Param>;

        struct FunPrototypeDecl : public Stmt {
            Name name;
            Path scopes;
            ParamList params;
            unique<Type> ret;
            FunPrototypeDecl(Name name, Path scopes, ParamList params, unique<Type> ret);
            std::string signature() const;
            SCAR_AST_ACCEPT_OVERRIDE
        };

        struct FunDecl : public Stmt {
            unique<FunPrototypeDecl> prototype;
            unique<Block> block;
            FunDecl(unique<FunPrototypeDecl> proto, unique<Block> blk);
            SCAR_AST_ACCEPT_OVERRIDE
        };

        struct ExprStmt : public Expr, public Stmt {
            unique<Expr> expr;
            ExprStmt(unique<Expr> e);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Module : public Node {
            std::vector<unique<Stmt>> stmts;
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

#define SCAR_AST_TYPE_DECL(name) \
    struct name : public Type { SCAR_AST_ACCEPT_OVERRIDE; }

        SCAR_AST_TYPE_DECL(StrType);
        SCAR_AST_TYPE_DECL(CharType);
        SCAR_AST_TYPE_DECL(BoolType);

        SCAR_AST_TYPE_DECL(I8Type);
        SCAR_AST_TYPE_DECL(I16Type);
        SCAR_AST_TYPE_DECL(I32Type);
        SCAR_AST_TYPE_DECL(I64Type);

        SCAR_AST_TYPE_DECL(U8Type);
        SCAR_AST_TYPE_DECL(U16Type);
        SCAR_AST_TYPE_DECL(U32Type);
        SCAR_AST_TYPE_DECL(U64Type);

        SCAR_AST_TYPE_DECL(F32Type);
        SCAR_AST_TYPE_DECL(F64Type);

        struct CustomType : public Type {
            Name name;
            CustomType(ast::Name name);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

#undef SCAR_AST_TYPE_DECL


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////


#undef SCAR_AST_ACCEPT_OVERRIDE
    }
}