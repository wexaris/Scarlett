#pragma once
#include "lex/lexer.hpp"

namespace scar {

    class Parser {

    private:
        Lexer lexer;
        Token tok;

        struct Flags {
            enum Stmt {
                NO_VAR    = 0x01,
                NO_STATIC = 0x02,
                NO_CONST  = 0x04,
                NO_FUN    = 0x08
            };
            const static Stmt STMT_GLOBAL = (Stmt)(Stmt::NO_VAR);
            const static Stmt STMT_BLOCK  = (Stmt)(
                Stmt::NO_STATIC |
                Stmt::NO_CONST  |
                Stmt::NO_FUN);
        };

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
        ast::Param param();
        std::vector<ast::Param> params();
        ast::ArgList arg_list();
        unique<ast::Block> block();

        unique<ast::Stmt> stmt(Flags::Stmt flags);
        unique<ast::VarDecl> var_decl();
        unique<ast::VarDecl> const_decl();
        unique<ast::VarDecl> static_decl();
        unique<ast::Stmt> fun_decl();
        unique<ast::FunPrototypeDecl> fun_prototype_decl();
        unique<ast::ExprStmt> expr_stmt();

        unique<ast::Expr> expr(unsigned int prec = 1);
        unique<ast::Expr> expr_atom(unsigned int prec);

        unique<ast::Type> type();

    public:
        Parser(std::string_view path);

        ast::Module parse();
    };

}