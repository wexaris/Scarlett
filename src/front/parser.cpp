#include "parser.hpp"
#include "log/logging.hpp"
#include "interner.hpp"
#include "err/error.hpp"
#include <unordered_map>
#include <stack>

namespace scar {

    namespace err_help {
        inline err::ParseError err_spanned(const Span& sp, std::string_view msg) {
            return err::ParseError::make(log::Error, "{}: {}", sp, msg);
        }
        inline err::ParseError err_unexpected(const Span& sp, std::string_view msg) {
            return err::ParseError::make(log::Error, "{}: unexpected {}", sp, msg);
        }
        inline err::ParseError err_unexpected_ty(const Token& tok) {
            return err_unexpected(tok.span, fmt::format("{}", ttype::to_str(tok.type)));
        }
    }

    void error_and_throw(const Token& tok, std::string_view msg) {
        err_help::err_spanned(tok.span, msg);
    }

    void error_and_throw(const Token& tok) {
        using namespace err_help;

        if (tok == END) {
            err_unexpected(tok.span, "end of file").emit();
        }
        else {
            err_unexpected_ty(tok).emit();
        }
    }

    Parser::Parser(std::string_view path) :
        lexer(path),
        tok(lexer.next_token())
    {}

    void Parser::failed_expect() {
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

    ast::Package Parser::parse() {
        ast::Package pack;

        while (!tok.is_eof()) {
            try {
                auto e = expr();
                (void)e;
            }
            catch (const err::RecoveryUnwind&) {
                error_count++;
                synchronize();
                log::info("recovered");
            }
        }
        
        return pack;
    }

    // path : ('::')? ident ('::' ident)*
    ast::Path Parser::path() {
        ast::Path p = {};
        ast::Name id;

        // Check for global scope
        if (match(Scope)) {
            bump();
        }

        // Parse first identifier
        // Parsing and errors will be easier if we loop on scopes,
        // not identifiers.
        id = tok.val_s;
        if (expect(Ident)) {
            p.push_back(id);
        }

        // Every next scope is expected to be followed by an identifier.
        // If it's not, we break the loop and error.
        while (match(Scope)) {
            bump();

            id = tok.val_s;
            if (expect(Ident)) {
                p.push_back(id);
            }
            else {
                break;
            }
        }

        return p;
    }

    unique<ast::Stmt> Parser::stmt() {
        return nullptr;
    }

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

    /* Constructs the appropriate function call expression.
    Differentiates normal functions from temporary pre-defined ones.
    TODO: should be removed in the future. */
    unique<ast::ExprFunCall> make_fun_call(ast::Path&& path, unique<ast::Expr> args) {
        if (path.size() == 1 && *path[0].get_str() == "print") {
            return std::make_unique<ast::ExprFunCallPrint>(std::move(args));
        }
        return std::make_unique<ast::ExprFunCall>(path, std::move(args));
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

        case LitInteger:
            atom = std::make_unique<ast::Integer>(tok.val_i);
            bump();
            break;

        case LitFloat:
            atom = std::make_unique<ast::Float>(tok.val_f);
            bump();
            break;

        default:
            if (match(Scope) || match(Ident)) {
                auto id_path = path();

                // A '(' after an identifier is assumed to be a function call
                if (match(Lparen)) {
                    bump();
                    unique<ast::Expr> args;
                    if (!match(Rparen)) {
                        args = expr();
                    }
                    expect(Rparen);
                    atom = make_fun_call(std::move(id_path), std::move(args));
                }
                else {
                    atom = std::make_unique<ast::Var>(id_path);
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

}