#pragma once
#include "visitor/visitor.hpp"
#include <vector>
#include <string>
#include <memory>

namespace scar {

    template<typename T> using unique = std::unique_ptr<T>;
    template<typename T> using shared = std::shared_ptr<T>;
    template<typename T> using weak = std::weak_ptr<T>;
    using interned_str_t = std::shared_ptr<std::string>;

    namespace ast {

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

        struct Node {
            virtual ~Node() = default;
            virtual bool accept(ASTVisitor& v) = 0;
        };

#define SCAR_AST_ACCEPT_OVERRIDE \
    bool accept(ASTVisitor& v) override { \
        return v.visit(*this); \
    }	

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		struct Type : public Node {
			enum TypeKind {
				Unknown,
				StrType,
				CharType,
				BoolType,
				I8Type,
				I16Type,
				I32Type,
				I64Type,
				U8Type,
				U16Type,
				U32Type,
				U64Type,
				F32Type,
				F64Type,
				Custom,
				Void
			} kind;
			Name custom_name;

			Type() : kind(Unknown) {}
			Type(Name name) : kind(Custom), custom_name(name) {}
			Type(TypeKind kind) : kind(kind) {}
			SCAR_AST_ACCEPT_OVERRIDE;
		};


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		struct Expr : virtual public Node { Type type; };
		using expr_ptr = unique<Expr>;
		struct Stmt : virtual public Node {};
		using stmt_ptr = unique<Stmt>;


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

        struct BinOp : public Expr {
			expr_ptr lhs;
			expr_ptr rhs;

			enum Kind {
				Scope,
				MemberAccess,
				Sum,
				Sub,
				Mul,
				Div,
				Mod,
				Eq,
				NotEq,
				Greater,
				Lesser,
				GreaterEq,
				LesserEq,
				LogicAnd,
				LogicOr,
				BitAnd,
				BitOr,
				BitXOr,
				Shl,
				Shr,
				Assign,
				PlusAssign,
				MinusAssign,
				MulAssign,
				DivAssign,
				ModAssign,
				AndAssign,
				OrAssign,
				XorAssign,
				ShlAssign,
				ShrAssign,
			} kind;

            BinOp(expr_ptr lhs, expr_ptr rhs, Kind kind);
			SCAR_AST_ACCEPT_OVERRIDE;
		};

		struct UnaryOp : public Expr {
			expr_ptr expr;

			enum Kind {
				GlobalAccess,
				PreIncrement,
				PreDecrement,
				PostIncrement,
				PostDecrement,
				Positive,
				Negative,
				Not,
				BitNot,
				Deref,
				Address
			} kind;

			UnaryOp(expr_ptr expr, Kind kind);
			SCAR_AST_ACCEPT_OVERRIDE;
		};

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
            float val;
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

        using ArgList = std::vector<ast::expr_ptr>;

        struct FunCall : public Expr, public Stmt {
            Path path;
            ArgList args;
            FunCall(Path path, ArgList args);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Block : public Expr, public Stmt {
			std::vector<stmt_ptr> stmts;
            Block(std::vector<stmt_ptr> stmts);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        struct VarDecl : public Stmt {
            Name name;
            unique<Type> type;
            expr_ptr val;
            VarDecl(Name name, unique<Type> ty, expr_ptr val);
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
            ParamList params;
            unique<Type> ret;
            FunPrototypeDecl(Name name, ParamList params, unique<Type> ret);
            std::string signature() const;
            SCAR_AST_ACCEPT_OVERRIDE
        };

        struct FunDecl : public Stmt {
            unique<FunPrototypeDecl> prototype;
            unique<Block> block;
            FunDecl(unique<FunPrototypeDecl> proto, unique<Block> blk);
            SCAR_AST_ACCEPT_OVERRIDE
        };

		struct RetVoidStmt : public Stmt {
			RetVoidStmt() = default;
			SCAR_AST_ACCEPT_OVERRIDE;
		};

		struct RetStmt : public Stmt {
			expr_ptr expr;
			RetStmt(expr_ptr e) : expr(std::move(e)) {}
			SCAR_AST_ACCEPT_OVERRIDE;
		};

        struct ExprStmt : public Expr, public Stmt {
            expr_ptr expr;
            ExprStmt(expr_ptr e);
            SCAR_AST_ACCEPT_OVERRIDE;
        };

        struct Module : public Node {
            std::vector<stmt_ptr> stmts;
            SCAR_AST_ACCEPT_OVERRIDE;
        };


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////


#undef SCAR_AST_ACCEPT_OVERRIDE
    }
}

template<>
struct std::hash<scar::ast::Name> {
    inline size_t operator()(const scar::ast::Name& n) const {
        std::hash<size_t> hash;
        return hash(n.id);
    }
};

#include "fmt/format.h"

template<>
struct fmt::formatter<scar::ast::Name> {

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::ast::Name& n, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", n.get_strref());
    }
};