#include "parser.hpp"
#include "log/logging.hpp"
#include <unordered_map>
#include <stack>

namespace scar {

    Parser::Parser(std::string_view path) :
        lexer(path),
        tok(lexer.next_token())
    {}

    void Parser::bump() {
        tok = lexer.next_token();
    }

    inline void err_spanned(const Span& sp, std::string_view msg) {
        log::get_default()->error("{}: {}", sp, msg);
    }
    inline void err_unexpected(const Span& sp, std::string_view msg) {
        log::get_default()->error("{}: unexpected {}", sp, msg);
    }
    inline void err_unexpected_ty(const Token& tok) {
        err_unexpected(tok.span, fmt::format("{}", ttype::to_str(tok.type)));
    }

    bool Parser::check(TokenType tt) const {
        return tok.type == tt;
    }

    void Parser::expect(TokenType tt) {
        if (check(tt)) {
            bump();
            return;
        }
        err_unexpected_ty(tok);
        synchronize();
    }

    void Parser::synchronize() {
        while (!check(Semi)) {
            bump();
        }
        bump();
    }

    ast::Package Parser::parse() {
        ast::Package pack;

        while (!tok.is_eof()) {
            auto e = expr();
            (void)e;
        }
        
        return pack;
    }

    unique<ast::Stmt> Parser::stmt() {
        return nullptr;
    }

    /* Contains some of the helper structures and functions used for
    correct operator handling while parsing expressions. */
    namespace op {

        constexpr bool is_post_unaryop(TokenType ty) {
            return
                ty == PlusPlus ||
                ty == MinusMinus;
        }
        constexpr bool is_pre_unaryop(TokenType ty) {
            return
                ty == Plus ||
                ty == Minus ||
                ty == Not ||
                ty == BitNot;
        }
        constexpr bool is_binop(TokenType ty) {
            return
                ty == Scope ||
                ty == Dot ||
                ty == Plus ||
                ty == Minus ||
                ty == Star ||
                ty == Slash ||
                ty == Percent ||
                ty == LogicAnd ||
                ty == LogicOr ||
                ty == BitAnd ||
                ty == BitOr ||
                ty == Caret ||
                ty == Shl ||
                ty == Shr;
        }        

        struct OpInfo {
            unsigned int prec;
            enum { LEFT, RIGHT } assoc;
        };
        static const std::unordered_map<TokenType, OpInfo> post_unaryop_info_map{
            { PlusPlus,     OpInfo{ 10, OpInfo::LEFT } },
            { MinusMinus,   OpInfo{ 10, OpInfo::LEFT } },
        };
        static const std::unordered_map<TokenType, OpInfo> pre_unaryop_info_map{
            { PlusPlus,     OpInfo{ 9, OpInfo::RIGHT } },
            { MinusMinus,   OpInfo{ 9, OpInfo::RIGHT } },
            { Plus,         OpInfo{ 9, OpInfo::RIGHT } },
            { Minus,        OpInfo{ 9, OpInfo::RIGHT } },
            { Not,          OpInfo{ 9, OpInfo::RIGHT } },
            { BitNot,       OpInfo{ 9, OpInfo::RIGHT } },
            { Star,         OpInfo{ 9, OpInfo::RIGHT } },
            { BitAnd,       OpInfo{ 9, OpInfo::RIGHT } },
        };
        static const std::unordered_map<TokenType, OpInfo> binop_info_map{
            { Scope,    OpInfo{ 11, OpInfo::LEFT } },
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
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_post_unary_op(const Token& tok, unique<ast::Expr> atom) {

#define SCAR_MATCH_TYPE_OPERATOR_CASE(tok_ty, ast_op) \
    case tok_ty: return std::make_unique<ast_op>(std::move(atom));

            switch (tok.type)
            {
                SCAR_MATCH_TYPE_OPERATOR_CASE(PlusPlus, ast::ExprPostIncrement);
                SCAR_MATCH_TYPE_OPERATOR_CASE(MinusMinus, ast::ExprPostDecrement);
            default:
                err_unexpected_ty(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

        /* Construct the expropriate operator AST node for the token's type.
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_pre_unary_op(const Token& tok, unique<ast::Expr> atom) {
            
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
                err_unexpected_ty(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

        /* Construct the expropriate operator AST node for the token's type.
        We use a full token instead of a type to get more accurate error messages. */
        unique<ast::Expr> make_binop(const Token& tok, unique<ast::Expr> lhs, unique<ast::Expr> rhs) {

#define SCAR_MATCH_TYPE_OPERATOR_CASE(tok_ty, ast_op) \
    case tok_ty: return std::make_unique<ast_op>(std::move(lhs), std::move(rhs));

            switch (tok.type)
            {
                SCAR_MATCH_TYPE_OPERATOR_CASE(Scope, ast::ExprScope);
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
                err_unexpected_ty(tok);
                return nullptr;
            }
#undef SCAR_MATCH_TYPE_OPERATOR_CASE
        }

    }

    unique<ast::Expr> Parser::expr(unsigned int prec) {
        unique<ast::Expr> lhs = expr_atom(prec);

        while (op::is_binop(tok.type)) {
            // Find the corresponding OpInfo
            auto info = op::binop_info_map.at(tok.type);
            if (info.prec < prec) {
                break;
            }
            auto op_tok = tok;
            bump();

            unique<ast::Expr> rhs;
            if (info.assoc == op::OpInfo::LEFT) {
                rhs = expr(info.prec + 1);
            }
            else {
                rhs = expr(info.prec);
            }

            lhs = op::make_binop(op_tok, std::move(lhs), std::move(rhs));
        }

        return lhs;
    }

    /* Constructs the appropriate function call expression.
    Differentiates normal functions from temporary pre-defined ones.
    TODO: should be removed in the future. */
    unique<ast::ExprFunCall> make_fun_call(ast::Name name, unique<ast::Expr> args) {
        if (*name.get_str() == "print") {
            return std::make_unique<ast::ExprFunCallPrint>(std::move(args));
        }
        return std::make_unique<ast::ExprFunCall>(name, std::move(args));
    }

    unique<ast::Expr> Parser::expr_atom(unsigned int prec) {
        unique<ast::Expr> atom;

        if (op::is_pre_unaryop(tok.type)) {
            auto info = op::pre_unaryop_info_map.at(tok.type);
            if (info.prec >= prec) {
                auto op_tok = tok;
                bump();

                if (info.assoc == op::OpInfo::LEFT) {
                    atom = expr(info.prec + 1);
                }
                else {
                    atom = expr(info.prec);
                }
                atom = op::make_pre_unary_op(op_tok, std::move(atom));
                return atom;
            }
        }

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

        case Ident:
        {
            auto id_name = tok.val_s;
            bump();
            // A '(' after an identifier is assumed to be a function call
            if (check(Lparen)) {
                bump();
                unique<ast::Expr> args;
                if (!check(Rparen)) {
                    args = expr();
                }
                expect(Rparen);
                atom = make_fun_call(tok.val_s, std::move(args));
            }
            atom = std::make_unique<ast::Var>(id_name);
            break;
        }

        case END:
            err_unexpected(tok.span, "end of file");
            break;

        default:
            err_unexpected_ty(tok);
            synchronize();
            break;
        }

        while (op::is_post_unaryop(tok.type)) {
            auto info = op::post_unaryop_info_map.at(tok.type);
            if (
                info.prec >= prec) {
                atom = op::make_post_unary_op(tok, std::move(atom));
                bump();
            }
        }

        return atom;
    }

}