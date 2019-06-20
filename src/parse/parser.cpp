#include "parser.hpp"
#include "driver/session.hpp"
#include "error/error.hpp"
#include <unordered_map>
#include <stack>

namespace scar {

    namespace err_help {
        inline err::ParseError log_spanned(log::Level lvl, const Span& sp, std::string_view msg) {
            return err::ParseError::make(lvl, "{}: {}", sp, msg);
        }
        inline err::ParseError err_unexpected(const Span& sp, std::string_view msg) {
            return log_spanned(log::Level::Error, sp, fmt::format("unexpected {}", msg));
        }
        inline err::ParseError err_unexpected_ty(const Token& tok) {
            return err_unexpected(tok.span, ttype::to_str(tok.type));
        }
    }

    Parser::Parser(std::string_view path) :
        lexer(path),
        tok(lexer.next_token())
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
        tok = lexer.next_token();
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
        std::vector<unique<ast::Stmt>> stmts;

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
    unique<ast::Stmt> Parser::stmt(Flags::Stmt flags) {
        unique<ast::Stmt> ret;
        switch (tok.type)
        {
        case Semi:
            bump();
            ret = stmt(flags);
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

        unique<ast::Expr> val;
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

        unique<ast::Expr> val;
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
    unique<ast::Stmt> Parser::fun_decl() {
        auto prototype = fun_prototype_decl();

        if (!match(Lbrace)) {
            return prototype;
        }
        auto blk = block();

        return static_cast<unique<ast::Stmt>>(
            std::make_unique<ast::FunDecl>(std::move(prototype), std::move(blk)));
    }

    // fun_prototype_decl : FUN ident fun_params (RARROW type)?
    unique<ast::FunPrototypeDecl> Parser::fun_prototype_decl() {
        expect(Fun);
        auto scopes = path();
        auto name = scopes.back();
        scopes.pop_back();

        auto parameters = params();
        
        unique<ast::Type> ret;
        if (match(Rarrow)) {
            bump();
            ret = type();
        }

        return std::make_unique<ast::FunPrototypeDecl>(name, std::move(scopes), std::move(parameters), std::move(ret));
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

        constexpr bool is_preop(TokenType ty) {
            return
                ty == PlusPlus || ty == MinusMinus ||
                ty == Plus     || ty == Minus ||
                ty == Not      || ty == BitNot ||
                ty == Star     || ty == BitAnd;
        }
        constexpr bool is_postop(TokenType ty) {
            return ty == PlusPlus || ty == MinusMinus;
        }
        constexpr bool is_binop(TokenType ty) {
            return ty == Scope || ty == Dot     ||
                ty == Plus     || ty == Minus   ||
                ty == Star     || ty == Slash   || ty == Percent ||
                ty == LogicAnd || ty == LogicOr ||
                ty == BitAnd   || ty == BitOr   || ty == Caret ||
                ty == Shl      || ty == Shr;
        }        

        struct OpInfo {
            unsigned int prec;
            enum { LEFT, RIGHT } assoc;
        };
        static const std::unordered_map<TokenType, OpInfo> preop_info_map{
            { Scope,        OpInfo{ 12, OpInfo::RIGHT } },
            { PlusPlus,     OpInfo{ 9, OpInfo::RIGHT } },
            { PlusPlus,     OpInfo{ 9, OpInfo::RIGHT } },
            { MinusMinus,   OpInfo{ 9, OpInfo::RIGHT } },
            { Plus,         OpInfo{ 9, OpInfo::RIGHT } },
            { Minus,        OpInfo{ 9, OpInfo::RIGHT } },
            { Not,          OpInfo{ 9, OpInfo::RIGHT } },
            { BitNot,       OpInfo{ 9, OpInfo::RIGHT } },
            { Star,         OpInfo{ 9, OpInfo::RIGHT } },
            { BitAnd,       OpInfo{ 9, OpInfo::RIGHT } },
        };
        static const std::unordered_map<TokenType, OpInfo> postop_info_map{
            { PlusPlus,     OpInfo{ 10, OpInfo::LEFT } },
            { MinusMinus,   OpInfo{ 10, OpInfo::LEFT } },
        };
        static const std::unordered_map<TokenType, OpInfo> binop_info_map{
            { Dot,      OpInfo{ 10, OpInfo::LEFT } },
            { Star,     OpInfo{ 8, OpInfo::LEFT } },
            { Slash,    OpInfo{ 8, OpInfo::LEFT } },
            { Percent,  OpInfo{ 8, OpInfo::LEFT } },
            { Plus,     OpInfo{ 7, OpInfo::LEFT } },
            { Minus,    OpInfo{ 7, OpInfo::LEFT } },
            { Shl,      OpInfo{ 6, OpInfo::LEFT } },
            { Shr,      OpInfo{ 6, OpInfo::LEFT } },
            { BitAnd,   OpInfo{ 5, OpInfo::LEFT } },
            { Caret,    OpInfo{ 4, OpInfo::LEFT } },
            { BitOr,    OpInfo{ 3, OpInfo::LEFT } },
            { LogicAnd, OpInfo{ 2, OpInfo::LEFT } },
            { LogicOr,  OpInfo{ 1, OpInfo::LEFT } },
        };

        /* Construct the expropriate operator AST node for the token's type.
        Emits an error message and begins recovery if the token isn't a valid operator.
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_preop(const Token& tok, unique<ast::Expr> atom) {

#define SCAR_MATCH_TYPE_OPERATOR_CASE(tok_ty, ast_op) \
    case tok_ty: return std::make_unique<ast_op>(std::move(atom));

            switch (tok.type)
            {
                SCAR_MATCH_TYPE_OPERATOR_CASE(PlusPlus, ast::ExprPreIncrement);
                SCAR_MATCH_TYPE_OPERATOR_CASE(MinusMinus, ast::ExprPreDecrement);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Minus, ast::ExprNegative);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Not, ast::ExprNot);
                SCAR_MATCH_TYPE_OPERATOR_CASE(BitNot, ast::ExprBitNot);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Star, ast::ExprDeref);
                SCAR_MATCH_TYPE_OPERATOR_CASE(BitAnd, ast::ExprAddress);
            case Plus:
                // There's no reason to wrap an expression
                // if it wouldn't be modified either way
                return atom;
            default:
                error_and_throw(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

        /* Construct the expropriate operator AST node for the token's type.
        Emits an error message and begins recovery if the token isn't a valid operator.
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_postop(const Token& tok, unique<ast::Expr> atom) {

#define SCAR_MATCH_TYPE_OPERATOR_CASE(tok_ty, ast_op) \
    case tok_ty: return std::make_unique<ast_op>(std::move(atom));

            switch (tok.type)
            {
                SCAR_MATCH_TYPE_OPERATOR_CASE(PlusPlus, ast::ExprPostIncrement);
                SCAR_MATCH_TYPE_OPERATOR_CASE(MinusMinus, ast::ExprPostDecrement);
            default:
                error_and_throw(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

        /* Construct the expropriate operator AST node for the token's type.
        Emits an error message and begins recovery if the token isn't a valid operator.
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_binop(const Token& tok, unique<ast::Expr> lhs, unique<ast::Expr> rhs) {

#define SCAR_MATCH_TYPE_OPERATOR_CASE(tok_ty, ast_op) \
    case tok_ty: return std::make_unique<ast_op>(std::move(lhs), std::move(rhs));

            switch (tok.type)
            {
                SCAR_MATCH_TYPE_OPERATOR_CASE(Dot, ast::ExprMemberAccess);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Plus, ast::ExprSum);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Minus, ast::ExprSub);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Star, ast::ExprMul);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Slash, ast::ExprDiv);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Percent, ast::ExprMod);
                SCAR_MATCH_TYPE_OPERATOR_CASE(LogicAnd, ast::ExprLogicAnd);
                SCAR_MATCH_TYPE_OPERATOR_CASE(LogicOr, ast::ExprLogicOr);
                SCAR_MATCH_TYPE_OPERATOR_CASE(BitAnd, ast::ExprBitAnd);
                SCAR_MATCH_TYPE_OPERATOR_CASE(BitOr, ast::ExprBitOr);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Caret, ast::ExprBitXOr);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Shl, ast::ExprShl);
                SCAR_MATCH_TYPE_OPERATOR_CASE(Shr, ast::ExprShr);
            default:
                error_and_throw(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

    }

    unique<ast::Expr> Parser::expr(unsigned int prec) {
        // Parse left side of the expression
        unique<ast::Expr> lhs = expr_atom(prec);

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
            unique<ast::Expr> rhs = expr(
                (info.assoc == op::OpInfo::LEFT) ? info.prec + 1 : info.prec);

            // Save the full expression to the left side,
            // so we can keep nesting expressions through loops
            lhs = op::make_binop(op_tok, std::move(lhs), std::move(rhs));
        }

        return lhs;
    }

    unique<ast::Expr> Parser::expr_atom(unsigned int prec) {
        unique<ast::Expr> atom;

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
            atom = std::make_unique<ast::I64Expr>(tok.val_i);
            bump();
            break;

        case LitFloat: // FIXME: separate float types
            atom = std::make_unique<ast::F64Expr>(tok.val_f);
            bump();
            break;

        default:
            if (match(Scope) || match(Ident)) {
                auto id_path = path();

                // A '(' after an identifier is assumed to be a function call
                if (match(Lparen)) {
                    auto args = arg_list();
                    atom = std::make_unique<ast::FunCall>(std::move(id_path), std::move(args));
                }
                else {
                    atom = std::make_unique<ast::VarExpr>(id_path.back()); // FIXME: handle full paths
                }
            }
            else {
                failed_expect();
            }
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
            return std::make_unique<ast::StrType>();
        case Char:
            bump();
            return std::make_unique<ast::CharType>();
        case Bool:
            bump();
            return std::make_unique<ast::BoolType>();

        case I8:
            bump();
            return std::make_unique<ast::I8Type>();
        case I16:
            bump();
            return std::make_unique<ast::I16Type>();
        case I32:
            bump();
            return std::make_unique<ast::I32Type>();
        case I64:
            bump();
            return std::make_unique<ast::I64Type>();

        case U8:
            bump();
            return std::make_unique<ast::I8Type>();
        case U16:
            bump();
            return std::make_unique<ast::I16Type>();
        case U32:
            bump();
            return std::make_unique<ast::I32Type>();
        case U64:
            bump();
            return std::make_unique<ast::I64Type>();

        case F32:
            bump();
            return std::make_unique<ast::F32Type>();
        case F64:
            bump();
            return std::make_unique<ast::F64Type>();

        case Ident:
            return std::make_unique<ast::CustomType>(expect(Ident).val_s);

        default:
            error_and_throw(tok);
            return nullptr;
        }
    }

}