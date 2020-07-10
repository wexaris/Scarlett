#pragma once
#include "Parse/Span.hpp"
#include "Parse/Interner.hpp"
#include <variant>

namespace scar {

    class Token {
    public:
        enum TokenType : uint16_t {
            Invalid = 0,

            Ident,

            // Literals
            LitInt,
            LitFloat,
            LitString,
            True,
            False,

            // Keywords
            Func,
            If,
            Else,
            For,
            While,
            Loop,
            Var,
            Break,
            Continue,
            Return,
            As,

            // Types
            Void,

            Bool,

            I8,
            I16,
            I32,
            I64,

            U8,
            U16,
            U32,
            U64,

            F32,
            F64,

            Char,
            String,

            // Symbols
            LParen,       // (
            RParen,       // )
            LBrace,       // {
            RBrace,       // }
            LBracket,     // [
            RBracket,     // ]

            Semi,         // ;
            Colon,        // :
            Scope,        // ::
            Comma,        // ,
            Dot,          // .
            RArrow,       // ->

            // Operators
            Not,          // !
            BitNot,       // ~
            PlusPlus,     // ++
            MinusMinus,   // --

            Eq,           // ==
            NotEq,        // !=
            Greater,      // >
            GreaterEq,    // >=
            Lesser,       // <
            LesserEq,     // <=

            Plus,         // +
            Minus,        // -
            Star,         // *
            Slash,        // /
            Percent,      // %
            LogicAnd,     // &&
            LogicOr,      // ||
            BitAnd,       // &
            BitOr,        // |
            BitXOr,       // ^

            Assign,       // =

            // EOF
            EndOfFile
        };

        const TokenType Type = Token::Invalid;
        const TokenType LiteralType = Token::Invalid; // Type=LitInt, LiteralType=i32
        const TextSpan Span;

        Token(const TextSpan& span);
        Token(TokenType type, const TextSpan& span);
        Token(TokenType type, TokenType literalType, uint64_t val, const TextSpan& span);
        Token(TokenType type, TokenType literalType, double val, const TextSpan& span);
        Token(TokenType type, std::string_view val, const TextSpan& span);

        TextPosition GetTextPos() const { return TextPosition(Span.Line, Span.Col, Span.Index); }
        uint64_t GetInt() const;
        double GetFloat() const;
        Interner::StringID GetName() const;
        const std::string& GetString() const;

        bool IsEOF() const { return Type == Token::EndOfFile; }
        bool IsValid() const { return Type != Token::Invalid; }

        // Get the corresponding source code fragment
        std::string_view GetRaw() const {
            return Span.File->GetString(Span.Index, Span.Length);
        }

        bool operator==(const Token::TokenType& type) const { return Type == type; }
        bool operator!=(const Token::TokenType& type) const { return Type != type; }

    private:
        const std::variant<uint64_t, double, Interner::StringID> m_Value;
    };

    std::string_view AsString(Token::TokenType type);
    std::string AsString(const Token& token);

    static std::ostream& operator<<(std::ostream& os, Token::TokenType tokenType) {
        return os << AsString(tokenType);
    }
    static std::ostream& operator<<(std::ostream& os, const Token& token) {
        return os << AsString(token);
    }

    class TokenStream {
    public:
        using iterator = std::vector<Token>::iterator;
        using const_iterator = std::vector<Token>::const_iterator;

        TokenStream() = default;

        void push_back(const Token& token) { m_Tokens.push_back(token); }

        Token& back()             { return m_Tokens.back(); }
        const Token& back() const { return m_Tokens.back(); }
        iterator begin()             { return m_Tokens.begin(); }
        iterator end()               { return m_Tokens.end(); }
        const_iterator begin() const { return m_Tokens.begin(); }
        const_iterator end() const   { return m_Tokens.end(); }

    private:
        std::vector<Token> m_Tokens;   
    };

}