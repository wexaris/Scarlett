#pragma once
#include "lex/lexer.hpp"

namespace scar {

    class Parser {

    private:
        Lexer lexer;
        Token tok;

        Parser(const Parser&) = delete;

        void operator=(const Parser&) = delete;

        bool check(TokenType tt) const;
        void expect(TokenType tt);
        void synchronize();
 
        void bump();

        unique<ast::Stmt> stmt();
        unique<ast::Expr> expr(unsigned int prec = 1);
        unique<ast::Expr> expr_atom(unsigned int prec);

    public:
        Parser(std::string_view path);

        ast::Package parse();
    };

}