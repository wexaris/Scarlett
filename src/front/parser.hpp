#pragma once
#include "lex/lexer.hpp"

namespace scar {

    class Parser {

    private:
        Lexer lexer;
        Token tok;
        unsigned int error_count = 0;

        Parser(const Parser&) = delete;

        void operator=(const Parser&) = delete;

        inline bool match(TokenType tt) const {
            return tok.type == tt;
        }
        template<typename... Types>
        inline bool match(TokenType t, Types... types) const {
            return match(t) ? true : match(types...);
        }

        template<typename... Types>
        inline Token expect(Types... types) {
            if (match(types...)) {
                auto t = std::move(tok);
                bump();
                return t;
            }
            else {
                failed_expect();
            }
        }

        [[noreturn]] void failed_expect();
        void synchronize();
 
        void bump();

        ast::Path path();
        ast::FunArgList arg_list();

        unique<ast::Stmt> stmt();
        unique<ast::ExprStmt> expr_stmt();
        unique<ast::FunCallPrint> print_stmt();

        unique<ast::Expr> expr(unsigned int prec = 1);
        unique<ast::Expr> expr_atom(unsigned int prec);

    public:
        Parser(std::string_view path);

        ast::Package parse();

        inline bool had_error() const { return error_count > 0; }
        inline unsigned int err_count() const { return error_count; }
    };

}