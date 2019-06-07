#pragma once
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
        struct Var : public Expr {
            Path path;
            Var(Path path) : path(std::move(path)) {}
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

        struct ExprStmt : public Expr, public Stmt {
            unique<Expr> expr;
            ExprStmt(unique<Expr> e) :
                expr(std::move(e))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        using FunArgList = std::vector<unique<ast::Expr>>;

        struct FunCall : public Expr, public Stmt {
            Path path;
            FunArgList args;
            FunCall(Path path, FunArgList args) :
                path(std::move(path)),
                args(std::move(args))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };
        struct FunCallPrint : public FunCall {
            FunCallPrint(FunArgList args);
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