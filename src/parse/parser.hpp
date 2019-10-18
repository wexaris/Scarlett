#pragma once
#include "lex/lexer.hpp"

namespace scar {

    class Parser {

    private:
		TokenStream tok_steam;
        Token tok;

        struct Flags {
            enum Stmt {
                NO_VAR    = 0x01,
                NO_STATIC = 0x02,
                NO_CONST  = 0x04,
                NO_FUN    = 0x08,
				NO_FLOW	  = 0x10
            };
            static constexpr int STMT_GLOBAL = (int)Stmt::NO_VAR | (int)Stmt::NO_FLOW;
            static constexpr int STMT_BLOCK  = 
                (int)Stmt::NO_STATIC |
				(int)Stmt::NO_CONST  |
				(int)Stmt::NO_FUN;
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
        
        // Get the next token
        void bump();

        ast::Path path();
        ast::Param param();
        std::vector<ast::Param> params();
        ast::ArgList arg_list();
        unique<ast::Block> block();

        ast::stmt_ptr stmt(int flags);
        unique<ast::VarDecl> var_decl();
        unique<ast::VarDecl> const_decl();
        unique<ast::VarDecl> static_decl();
        ast::stmt_ptr fun_decl();
        unique<ast::FunPrototypeDecl> fun_prototype_decl();
		unique<ast::RetStmt> ret_stmt();
		unique<ast::ExprStmt> expr_stmt();

        ast::expr_ptr expr(unsigned int prec = 1);
        ast::expr_ptr expr_atom(unsigned int prec);

        unique<ast::Type> type();

    public:
        Parser(TokenStream& ts);

        ast::Module parse();
    };

}