#pragma once
#include "token_type.hpp"
#include <vector>
#include <string>
#include <memory>

namespace scar {

    template<typename T> using unique = std::unique_ptr<T>;
    using interned_str_t = std::shared_ptr<std::string>;

    namespace ast {

        struct Node {
            virtual ~Node() = default;
        private:
            virtual void accept(struct ASTVisitor* v) = 0;
        };
        struct Stmt : virtual public Node {};
        struct Expr : virtual public Node {};

        struct ASTVisitor {
            virtual ~ASTVisitor() = default;
            virtual void visit(Node& node) {
                (void)node;
                printf("visited\n");
            }
        };

#define SCAR_AST_ACCEPT_OVERRIDE \
    void accept(ASTVisitor* v) override { \
        v->visit(*this); \
    }

        struct Name : public Node {
            size_t id;
            Name(size_t id = 0) : id(id) {}
            interned_str_t get_str() const;
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        using Path = std::vector<Name>;

        struct Integer : public Expr {
            size_t val;
            Integer(size_t val) : val(val) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct Float : public Expr {
            double val;
            Float(double val) : val(val) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct Ident : public Expr {
            Path path;
            Ident(Path path) : path(std::move(path)) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };


        struct UnaryOp : public Expr {
            unique<Expr> atom;
            UnaryOp(unique<Expr> atom) :
                atom(std::move(atom))
            {}
        };

#define SCAR_AST_UNARYOP_DECL(name, ...) \
    struct name : public UnaryOp { \
        name(unique<Expr> atom) : \
            UnaryOp(std::move(atom)) {} \
        SCAR_AST_ACCEPT_OVERRIDE; \
        __VA_ARGS__\
    }

        SCAR_AST_UNARYOP_DECL(ExprPostIncrement, );
        SCAR_AST_UNARYOP_DECL(ExprPostDecrement, );
        SCAR_AST_UNARYOP_DECL(ExprPreIncrement, );
        SCAR_AST_UNARYOP_DECL(ExprPreDecrement, );
        SCAR_AST_UNARYOP_DECL(ExprNegative, );
        SCAR_AST_UNARYOP_DECL(ExprNot, );
        SCAR_AST_UNARYOP_DECL(ExprBitNot, );
        SCAR_AST_UNARYOP_DECL(ExprDeref, );
        SCAR_AST_UNARYOP_DECL(ExprAddress, );

#undef SCAR_AST_UNARYOP_DECL


        struct BinOp : public Expr {
            unique<Expr> lhs;
            unique<Expr> rhs;
            BinOp(unique<Expr> lhs, unique<Expr> rhs) :
                lhs(std::move(lhs)),
                rhs(std::move(rhs))
            {}
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

        struct Type {
            enum TyTy {
                Custom,

                // Primitive
                Str = (int)TokenType::Str,
                Char = (int)TokenType::Char,
                Bool = (int)TokenType::Bool,

                Isize = (int)TokenType::Isize,
                I8 = (int)TokenType::I8,
                I16 = (int)TokenType::I16,
                I32 = (int)TokenType::I32,
                I64 = (int)TokenType::I64,

                Usize = (int)TokenType::Usize,
                U8 = (int)TokenType::U8,
                U16 = (int)TokenType::U16,
                U32 = (int)TokenType::U32,
                U64 = (int)TokenType::U64,

                F32 = (int)TokenType::F32,
                F64 = (int)TokenType::F64
            } type = Custom;
            Name custom;
            Type() = default;
            Type(TyTy t) : type(t) {}
            Type(TokenType t) : type(static_cast<TyTy>(t)) {}
            explicit Type(Name custom) : custom(custom) {}
            inline bool is_primitive() const { return type != Custom; }
        };

        struct VarDecl : public Stmt {
            Name name;
            Type type;
            unique<Expr> val;
            VarDecl(Name name, Type ty, unique<Expr> val) :
                name(name),
                type(ty),
                val(std::move(val))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Param {
            Name name;
            Type type;
        };

        struct FunPrototypeDecl : public Stmt {
            Name name;
            std::vector<Param> params;
            Type ret;
            FunPrototypeDecl(Name name, std::vector<Param> params, Type ret) :
                name(name),
                params(params),
                ret(ret)
            {}
            SCAR_AST_ACCEPT_OVERRIDE
        };

        using Block = std::vector<std::unique_ptr<Stmt>>;

        struct FunDecl : public Stmt {
            unique<FunPrototypeDecl> prototype;
            Block block;
            FunDecl(unique<FunPrototypeDecl> proto, Block blk) :
                prototype(std::move(proto)),
                block(std::move(blk))
            {}
            SCAR_AST_ACCEPT_OVERRIDE
        };

        struct ExprStmt : public Expr, public Stmt {
            unique<Expr> expr;
            ExprStmt(unique<Expr> e) :
                expr(std::move(e))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        using ArgList = std::vector<unique<ast::Expr>>;

        struct FunCall : public Expr, public Stmt {
            Path path;
            ArgList args;
            FunCall(Path path, ArgList args) :
                path(std::move(path)),
                args(std::move(args))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct FunCallPrint : public FunCall {
            FunCallPrint(ArgList args);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Module {
            std::vector<unique<Stmt>> stmts;
        };

        struct Package {
            std::vector<Module> mods;
        };

#undef SCAR_AST_ACCEPT_OVERRIDE
    }
}