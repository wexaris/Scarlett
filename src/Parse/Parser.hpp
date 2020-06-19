#pragma once
#include "Parse/AST/AST.hpp"
#include "Parse/Token.hpp"

namespace scar {

    class Parser {
    public:
        explicit Parser(const std::string& path);

        Ref<ast::Module> Parse();

    private:
        TokenStream m_TokenStream;
        TokenStream::iterator m_Token;

        Parser(const Parser&) = delete;
        void operator=(const Parser&) = delete;

        void Bump();
        TextSpan GetSpanFrom(const TextPosition& start) const {
            return TextSpan(m_Token->Span.File, start.Line, start.Col, start.Index, m_Token->Span.Index - start.Index);
        }

        bool Match(const std::vector<Token::TokenType>& expected) const;
        Token& Expect(const std::vector<Token::TokenType>& expected);
        void Synchronize(const std::vector<Token::TokenType>& delims);

        // Type
        Ref<ast::Type> Type();
        // Support
        ast::Ident Ident();
        ast::Arg Arg();
        // Declarations
        Ref<ast::Decl> Decl();
        Ref<ast::Decl> Function();
        Ref<ast::FunctionPrototype> FunctionPrototype();
        // Statements
        Ref<ast::Stmt> Stmt();
        Ref<ast::Branch> Branch();
        Ref<ast::ForLoop> ForLoop();
        Ref<ast::WhileLoop> WhileLoop();
        Ref<ast::WhileLoop> Loop();
        Ref<ast::Block> Block();
        Ref<ast::Continue> Continue();
        Ref<ast::Break> Break();
        Ref<ast::Return> Return();
        // Expressions
        Ref<ast::Expr> TryExpr();
        Ref<ast::Expr> Expr(unsigned int prec = 1, bool allowEmpty = false);
        Ref<ast::Expr> ExprAtom(unsigned int prec, bool allowEmpty = false);
        Ref<ast::Var> Var();

        bool IsPrefixOperator() const;
        bool IsSuffixOperator() const;
        bool IsBinaryOperator() const;
    };

}