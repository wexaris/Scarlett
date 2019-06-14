#pragma once
#include "token_type.hpp"
#include <vector>
#include <string>
#include <memory>

namespace scar {

    template<typename T> using unique = std::unique_ptr<T>;
    template<typename T> using shared = std::shared_ptr<T>;
    using interned_str_t = std::shared_ptr<std::string>;

    namespace ast {

        class Node {
            virtual void accept(struct ASTVisitor* v) = 0;
        public:
            virtual ~Node() = default;
        };
        struct Stmt : virtual public Node {};
        struct Expr : virtual public Node {};
        struct Type : public Node {};


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

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

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct Name {
            size_t id;
            Name(size_t id = 0) : id(id) {}
            Name(const Name& other) : id(other.id) {}
            interned_str_t get_str() const;
            bool operator==(const Name& other) const {
                return id == other.id;
            }
            inline bool operator!=(const Name& other) const {
                return !operator==(other);
            }
        };

        using Path = std::vector<Name>;


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

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


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

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


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

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

        struct Bool : public Expr {
            bool val;
            Bool(bool val) : val(val) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Ident : public Expr {
            Path path;
            Ident(Path path) : path(std::move(path)) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct VarDecl : public Stmt {
            Name name;
            unique<Type> type;
            unique<Expr> val;
            VarDecl(Name name, unique<Type> ty, unique<Expr> val) :
                name(name),
                type(std::move(ty)),
                val(std::move(val))
            {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Param {
            Name name;
            unique<Type> type;
            Param(Name name, unique<Type> type) :
                name(name),
                type(std::move(type))
            {}
        };

        struct FunPrototypeDecl : public Stmt {
            Name name;
            std::vector<Param> params;
            unique<Type> ret;
            FunPrototypeDecl(Name name, std::vector<Param> params, unique<Type> ret) :
                name(name),
                params(std::move(params)),
                ret(std::move(ret))
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
            CustomType(ast::Name name) : name(name) {}
            SCAR_AST_ACCEPT_OVERRIDE;
        };

#undef SCAR_AST_TYPE_DECL


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////


#undef SCAR_AST_ACCEPT_OVERRIDE
    }
}