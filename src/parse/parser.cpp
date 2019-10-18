#include "parser.hpp"
#include "driver/session.hpp"
#include "util/error.hpp"
#include <unordered_map>
#include <stack>

namespace scar {

    namespace err_help {
        inline err::ParseError log_spanned(log::Level lvl, const Span& sp, std::string_view msg) {
            return err::ParseError::make(lvl, "{}: {}", sp, msg);
        }
        inline err::ParseError err_unexpected(const Span& sp, std::string_view msg) {
            return log_spanned(log::Level::Error, sp, FMT("unexpected {}", msg));
        }
        inline err::ParseError err_unexpected_ty(const Token& tok) {
            return err_unexpected(tok.span, ttype::to_str(tok.type));
        }
    }

    Parser::Parser(TokenStream& ts) :
        tok_steam(ts),
		tok(tok_steam.get())
    {}

    [[noreturn]] void error_and_throw(const Token& tok) {
        using namespace err_help;

        if (tok == END) {
            err_unexpected(tok.span, "end of file").emit();
        }
        else {
            err_unexpected_ty(tok).emit();
        }
    }

    [[noreturn]] void Parser::failed_expect() {
        error_and_throw(tok);
    }



    // Contains all of the TokenTypes that could start a statement.
    // These can be used as synchronization points in error recovery,
    constexpr TokenType stmt_start[]{
        Var,
        Fun,
        Struct,
        Enum,
        Union,
        Trait,
        Macro,

        If,
        Else,
        Loop,
        For,
        Do,
        While,
        Switch,
        Case,
        Match,
        Continue,
        Break,
        Return,

        // Attributes
        Pub,
        Priv,
        Static,
        Const,
        Mut,
    };

    void Parser::synchronize() {
        while (!tok.is_eof()) {
            // Sync to the ending of the broken code
            if (match(Semi)) {
                bump();
                break;
            }

            // Sync to the beginning of a new statement
            for (auto& ty : stmt_start) {
                if (tok == ty) {
                    break;
                }
            }

            bump();
        }
    }

    void Parser::bump() {
        tok_steam++;
		tok = tok_steam.get();
    }

    ast::Module Parser::parse() {
        ast::Module module;

        while (!tok.is_eof()) {
            try {
                 module.stmts.push_back(stmt(Flags::STMT_GLOBAL));
            }
            catch (const err::RecoveryUnwind&) {
                synchronize();
            }
        }
        
        return module;
    }

    // path : ('::')? ident ('::' ident)*
    ast::Path Parser::path() {
        ast::Path p = {};
        Token t;

        // Check for global scope
        if (match(Scope)) {
            bump();
        }

        // Parse first identifier
        // Parsing and errors will be easier if we loop on scopes,
        // not identifiers.
        t = expect(Ident);
        p.push_back(t.val_s);

        // Every next scope is expected to be followed by an identifier.
        // If it's not, we break the loop and error.
        while (match(Scope)) {
            bump();

            t = expect(Ident);
            p.push_back(t.val_s);
        }

        return p;
    }

    // param : ident ':' type
    ast::Param Parser::param() {
        auto name = expect(Ident).val_s;
        expect(Col);
        auto ty = type();
        return ast::Param{ name, std::move(ty) };
    }

    // params : '(' param (',' param)* (',')? ')'
    ast::ParamList Parser::params() {
        ast::ParamList params;

        expect(Lparen);

        // Keep reading expressions, as long as there are commas
        // Allow a trailing comma with no following expression
        if (tok != Rparen) {
            params.push_back(param());
            while (tok == Comma) {
                bump();
                if (tok == Rparen) {
                    break;
                }
                params.push_back(param());
            }
        }

        expect(Rparen);

        return params;
    }

    // arg_list : '(' expr (',' expr)* (',')? ')'
    ast::ArgList Parser::arg_list() {
        ast::ArgList args = {};

        expect(Lparen);

        // Keep reading expressions, as long as there are commas
        // Allow a trailing comma with no following expression
        if (tok != Rparen) {
            args.push_back(expr());
            while (tok == Comma) {
                bump();
                if (tok == Rparen) {
                    break;
                }
                args.push_back(expr());
            }
        }

        expect(Rparen);

        return args;
    }

    // block : '{' stmt* '}'
    unique<ast::Block> Parser::block() {
        std::vector<ast::stmt_ptr> stmts;

        expect(Lbrace);

        while (!match(Rbrace)) {
            stmts.push_back(stmt(Flags::STMT_BLOCK));
        }

        expect(Rbrace);

        return std::make_unique<ast::Block>(std::move(stmts));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // stmt : expr_stmt
    //      | print_stmt
    ast::stmt_ptr Parser::stmt(int flags) {
        ast::stmt_ptr ret;
        switch (tok.type)
        {
        case Semi:
            bump();
            break;
        case Var:
            ret = var_decl();
            if (flags & Flags::Stmt::NO_VAR) {

            }
            break;
        case Static:
            ret = static_decl();
            if (flags & Flags::Stmt::NO_STATIC) {

            }
            break;
        case Const:
            ret = const_decl();
            if (flags & Flags::Stmt::NO_CONST) {

            }
            break;
        case Fun:
            ret = fun_decl();
            if (flags & Flags::Stmt::NO_FUN) {
				Session::get().logger().make_error("function declarations not allowed here");
            }
            break;
		case Return:
			ret = ret_stmt();
			if (flags & Flags::Stmt::NO_FLOW) {
				Session::get().logger().make_error("function declarations not allowed here");
			}
			break;
        default:
            ret = expr_stmt();
            break;
        }
        return ret;
    }

    // var_decl : VAR ident ':' type ('=' expr)? ';'
    unique<ast::VarDecl> Parser::var_decl() {
        expect(Var);
        auto name = expect(Ident).val_s;

        expect(Col);
        auto ty = type();

        ast::expr_ptr val;
        if (match(Assign)) {
            bump();
            val = expr();
        }
        expect(Semi);

        return std::make_unique<ast::VarDecl>(name, std::move(ty), std::move(val));
    }

    // static_decl : STATIC ident ':' type ('=' expr)? ';'
    unique<ast::VarDecl> Parser::static_decl() {
        expect(Static);
        auto name = expect(Ident).val_s;

        expect(Col);
        auto ty = type();

        ast::expr_ptr val;
        if (match(Assign)) {
            bump();
            val = expr();
        }
        expect(Semi);

        return std::make_unique<ast::VarDecl>(name, std::move(ty), std::move(val));
    }

    // const_decl : CONST ident ':' type '=' expr ';'
    unique<ast::VarDecl> Parser::const_decl() {
        expect(Const);
        auto name = expect(Ident).val_s;

        expect(Col);
        auto ty = type();

        expect(Assign);
        auto val = expr();

        expect(Semi);

        return std::make_unique<ast::VarDecl>(name, std::move(ty), std::move(val));
    }

    // fun_decl : fun_prototype_decl
    //          | fun_prototype_decl block
    ast::stmt_ptr Parser::fun_decl() {
        auto prototype = fun_prototype_decl();

        if (!match(Lbrace)) {
			expect(Semi);
            return prototype;
        }
        auto blk = block();

        return static_cast<ast::stmt_ptr>(
            std::make_unique<ast::FunDecl>(std::move(prototype), std::move(blk)));
    }

    // fun_prototype_decl : FUN ident params (RARROW type)?
    unique<ast::FunPrototypeDecl> Parser::fun_prototype_decl() {
        expect(Fun);
        auto name = expect(Ident).val_s;

        auto parameters = params();
        
        unique<ast::Type> ret;
        if (match(Rarrow)) {
            bump();
            ret = type();
        }
		else {
			ret = std::make_unique<ast::Type>(ast::Type::Void);
		}

        return std::make_unique<ast::FunPrototypeDecl>(name, std::move(parameters), std::move(ret));
    }

	// ret_stmt : RETURN expr ';'
	unique<ast::RetStmt> Parser::ret_stmt() {
		expect(Return);
		ast::expr_ptr ex;
		if (!match(Semi)) { ex = expr(); }
		expect(Semi);
		return std::make_unique<ast::RetStmt>(std::move(ex));
	}

    // expr_stmt : expr ';'
    unique<ast::ExprStmt> Parser::expr_stmt() {
        auto e = expr();
        expect(Semi);
        return std::make_unique<ast::ExprStmt>(std::move(e));
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    /* Contains some of the helper structures and functions used for
    correct operator handling while parsing expressions. */
    namespace op {

		struct OpInfo {
			unsigned int prec;
			enum Assoc { LEFT, RIGHT } assoc;

			OpInfo(unsigned int prec, Assoc assoc, ast::UnaryOp::Kind op) : prec(prec), assoc(assoc), opcode((int)op) {}
			OpInfo(unsigned int prec, Assoc assoc, ast::BinOp::Kind op) : prec(prec), assoc(assoc), opcode((int)op) {}

			inline ast::UnaryOp::Kind as_unaryop() const { return static_cast<ast::UnaryOp::Kind>(opcode); }
			inline ast::BinOp::Kind as_binop() const { return static_cast<ast::BinOp::Kind>(opcode); }

		private:
			int opcode;
		};
		static const std::unordered_map<TokenType, OpInfo> preop_info_map{
			{ Scope,        OpInfo{ 14, OpInfo::RIGHT, ast::UnaryOp::GlobalAccess } },
			{ PlusPlus,     OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::PreIncrement } },
			{ MinusMinus,   OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::PreDecrement } },
			{ Plus,         OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::Positive } },
			{ Minus,        OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::Negative } },
			{ Not,          OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::Not } },
			{ BitNot,       OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::BitNot } },
			{ Star,         OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::Deref } },
			{ BitAnd,       OpInfo{ 12, OpInfo::RIGHT, ast::UnaryOp::Address } },
		};
		static const std::unordered_map<TokenType, OpInfo> postop_info_map{
			{ PlusPlus,     OpInfo{ 13, OpInfo::LEFT, ast::UnaryOp::PostIncrement } },
			{ MinusMinus,   OpInfo{ 13, OpInfo::LEFT, ast::UnaryOp::PostDecrement } },
		};
		static const std::unordered_map<TokenType, OpInfo> binop_info_map{
			{ Scope,		OpInfo{ 14, OpInfo::LEFT, ast::BinOp::Scope } },
			{ Dot,			OpInfo{ 13, OpInfo::LEFT, ast::BinOp::MemberAccess } },
			{ Star,			OpInfo{ 11, OpInfo::LEFT, ast::BinOp::Mul } },
			{ Slash,		OpInfo{ 11, OpInfo::LEFT, ast::BinOp::Div } },
			{ Percent,		OpInfo{ 11, OpInfo::LEFT, ast::BinOp::Mod } },
			{ Plus,			OpInfo{ 10, OpInfo::LEFT, ast::BinOp::Sum } },
			{ Minus,		OpInfo{ 10, OpInfo::LEFT, ast::BinOp::Sub } },
			{ Shl,			OpInfo{ 9, OpInfo::LEFT, ast::BinOp::Shl } },
			{ Shr,			OpInfo{ 9, OpInfo::LEFT, ast::BinOp::Shr } },
			{ Greater,		OpInfo{ 8, OpInfo::LEFT, ast::BinOp::Greater } },
			{ Lesser,		OpInfo{ 8, OpInfo::LEFT, ast::BinOp::Lesser } },
			{ GreaterEq,	OpInfo{ 8, OpInfo::LEFT, ast::BinOp::GreaterEq } },
			{ LesserEq,		OpInfo{ 8, OpInfo::LEFT, ast::BinOp::LesserEq } },
			{ Eq,			OpInfo{ 7, OpInfo::LEFT, ast::BinOp::Eq } },
			{ NotEq,		OpInfo{ 7, OpInfo::LEFT, ast::BinOp::NotEq } },
			{ BitAnd,		OpInfo{ 6, OpInfo::LEFT, ast::BinOp::BitAnd } },
			{ Caret,		OpInfo{ 5, OpInfo::LEFT, ast::BinOp::BitXOr } },
			{ BitOr,		OpInfo{ 4, OpInfo::LEFT, ast::BinOp::BitOr } },
			{ LogicAnd,		OpInfo{ 3, OpInfo::LEFT, ast::BinOp::LogicAnd } },
			{ LogicOr,		OpInfo{ 2, OpInfo::LEFT, ast::BinOp::LogicOr } },
			{ Assign,		OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::Assign } },
			{ PlusAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::PlusAssign } },
			{ MinusAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::MinusAssign } },
			{ MulAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::MulAssign } },
			{ DivAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::DivAssign } },
			{ ModAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::ModAssign } },
			{ AndAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::AndAssign } },
			{ OrAssign,		OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::OrAssign } },
			{ XorAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::XorAssign } },
			{ ShlAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::ShlAssign } },
			{ ShrAssign,	OpInfo{ 1, OpInfo::RIGHT, ast::BinOp::ShrAssign } }
		};

        inline bool is_preop(TokenType ty)	{ return preop_info_map.find(ty) != preop_info_map.end(); }
		inline bool is_postop(TokenType ty) { return postop_info_map.find(ty) != postop_info_map.end(); }
		inline bool is_binop(TokenType ty)	{ return binop_info_map.find(ty) != binop_info_map.end(); }        

        /* Construct the expropriate AST UnaryOp node for the token's type. */
        inline ast::expr_ptr make_preop(const Token& tok, ast::expr_ptr expr) {
			if (tok.type == Plus) { return expr; } // Optimize away the positive unary operator node
			return std::make_unique<ast::UnaryOp>(std::move(expr), preop_info_map.at(tok.type).as_unaryop());
        }
		/* Construct the expropriate AST UnaryOp node for the token's type. */
		inline ast::expr_ptr make_postop(const Token& tok, ast::expr_ptr expr) {
			return std::make_unique<ast::UnaryOp>(std::move(expr), postop_info_map.at(tok.type).as_unaryop());
		}
		/* Construct the expropriate AST BinOp node for the token's type. */
		inline ast::expr_ptr make_binop(const Token& tok, ast::expr_ptr lhs, ast::expr_ptr rhs) {
			return std::make_unique<ast::BinOp>(std::move(lhs), std::move(rhs), binop_info_map.at(tok.type).as_binop());
        }

    }

    ast::expr_ptr Parser::expr(unsigned int prec) {
        // Parse left side of the expression
        ast::expr_ptr lhs = expr_atom(prec);

        while (op::is_binop(tok.type)) {
            // Find the corresponding OpInfo.
            // If the operators precedence is smaller than the current one,
            // return without parsing the right side.
            auto info = op::binop_info_map.at(tok.type);
            if (info.prec < prec) {
                break;
            }
            auto op_tok = tok;
            bump();

            // Parse right side of the expression
            // If the operator was left associative, increase the precedence
            ast::expr_ptr rhs = expr(
                (info.assoc == op::OpInfo::LEFT) ? info.prec + 1 : info.prec);

            // Save the full expression to the left side,
            // so we can keep nesting expressions through loops
            lhs = op::make_binop(op_tok, std::move(lhs), std::move(rhs));
        }

        return lhs;
    }

    ast::expr_ptr Parser::expr_atom(unsigned int prec) {
        ast::expr_ptr atom;

        // Parse a prefix unary operator
        if (op::is_preop(tok.type)) {
            auto info = op::preop_info_map.at(tok.type);
            if (info.prec >= prec) {
                auto op_tok = tok;
                bump();

                // Parse the expression that will be operated on
                // If the operator was left associative, increase the precedence
                atom = expr(
                    (info.assoc == op::OpInfo::LEFT) ? info.prec + 1 : info.prec);

                // Return with the operator's AST node
                return op::make_preop(op_tok, std::move(atom));
            }
        }

        // Parse main expression
        switch (tok.type)
        {
        case Lparen:
            bump();
            atom = expr();
            expect(Rparen);
            break;

        case LitBool:
            atom = std::make_unique<ast::BoolExpr>(static_cast<int8_t>(tok.val_i));
            bump();
            break;

        case LitInteger: // FIXME: separate int types
            atom = std::make_unique<ast::I32Expr>(tok.val_i);
            bump();
            break;

        case LitFloat: // FIXME: separate float types
            atom = std::make_unique<ast::F32Expr>(tok.val_f);
            bump();
            break;

        default: // FIXME: separate variables, structs, namespaces
			atom = std::make_unique<ast::VarExpr>(expect(Ident).val_s);
            break;
        }

        // Parse a postfix unary operator
        while (op::is_postop(tok.type)) {
            auto info = op::postop_info_map.at(tok.type);
            if (info.prec >= prec) {
                auto op_tok = tok;
                bump();

                // Return with the operator's AST node
                return op::make_postop(op_tok, std::move(atom));
            }
        }

        return atom;
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // ty : ident
    //    | STR | CHAR | BOOL
    //    | ISIZE | I8 | I16 | I32 | I64
    //    | USIZE | U8 | U16 | U32 | U64
    //    | F32 | F64
    unique<ast::Type> Parser::type() {
        switch (tok.type)
        {
        case Str:
            bump();
            return std::make_unique<ast::Type>(ast::Type::StrType);
        case Char:
            bump();
            return std::make_unique<ast::Type>(ast::Type::CharType);
        case Bool:
            bump();
            return std::make_unique<ast::Type>(ast::Type::BoolType);

        case I8:
            bump();
            return std::make_unique<ast::Type>(ast::Type::I8Type);
        case I16:
            bump();
            return std::make_unique<ast::Type>(ast::Type::I16Type);
        case I32:
            bump();
            return std::make_unique<ast::Type>(ast::Type::I32Type);
        case I64:
            bump();
            return std::make_unique<ast::Type>(ast::Type::I64Type);

        case U8:
            bump();
            return std::make_unique<ast::Type>(ast::Type::U8Type);
        case U16:
            bump();
			return std::make_unique<ast::Type>(ast::Type::U16Type);
		case U32:
            bump();
            return std::make_unique<ast::Type>(ast::Type::U32Type);
        case U64:
            bump();
            return std::make_unique<ast::Type>(ast::Type::U64Type);

        case F32:
            bump();
            return std::make_unique<ast::Type>(ast::Type::F32Type);
        case F64:
            bump();
            return std::make_unique<ast::Type>(ast::Type::F64Type);

        case Ident:
            return std::make_unique<ast::Type>(expect(Ident).val_s);

        default:
            error_and_throw(tok);
            return nullptr;
        }
    }

}