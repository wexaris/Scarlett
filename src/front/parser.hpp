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
        inline bool expect(Types... types) {
            if (match(types...)) {
                bump();
                return true;
            }
            failed_expect();
            return false;
        }

        void failed_expect();
        void synchronize();
 
        void bump();

        ast::Path path();

        unique<ast::Stmt> stmt();
        unique<ast::Expr> expr(unsigned int prec = 1);
        unique<ast::Expr> expr_atom(unsigned int prec);

    public:
        Parser(std::string_view path);

        ast::Package parse();

        inline bool had_error() const { return error_count > 0; }
        inline unsigned int err_count() const { return error_count; }
    };

}