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
        const TokenType ValueType = Token::Invalid;
        const TextSpan Span;

        Token(const TextSpan& span) :
            Type(Invalid), Span(span) {}
        Token(TokenType type, const TextSpan& span) :
            Type(type), Span(span) {}
        Token(TokenType type, TokenType valType, uint64_t val, const TextSpan& span) :
            Type(type), Span(span), m_Value(std::in_place_index_t<0>{}, val), ValueType(valType) {}
        Token(TokenType type, TokenType valType, double val, const TextSpan& span) :
            Type(type), Span(span), m_Value(std::in_place_index_t<1>{}, val), ValueType(valType) {}
        Token(TokenType type, std::string_view val, const TextSpan& span) :
            Type(type), Span(span), m_Value(std::in_place_index_t<2>{}, Interner::Intern(val)), ValueType(String) {}

        TextPosition GetTextPos() const { return TextPosition(Span.Line, Span.Col, Span.Index); }
        uint64_t GetInt() const {
            SCAR_ASSERT(Type == LitInt, "trying to get integer value from non-integer token!");
            return std::get<0>(m_Value);
        }
        double GetFloat() const {
            SCAR_ASSERT(Type == LitFloat, "trying to get floating point value from non-float Token!");
            return std::get<1>(m_Value);
        }
        Interner::StringID GetName() const {
            SCAR_ASSERT(Type == LitString || Type == Ident, "trying to get StringID from non-string Token!");
            return std::get<2>(m_Value);
        }
        std::string_view GetString() const { return Interner::GetString(GetName()); }

        bool IsEOF() const { return Type == Token::EndOfFile; }
        bool IsValid() const { return Type != Token::Invalid; }

        // Get the corresponding source code fragment
        std::string_view GetRaw() const {
            return Span.File->GetString(Span.Index, Span.Length);
        }

        bool operator==(const TokenType& type) const { return Type == type; }
        bool operator!=(const TokenType& type) const { return Type != type; }

    private:
        const std::variant<uint64_t, double, Interner::StringID> m_Value;
    };

    static std::string_view AsString(Token::TokenType type) {
        switch (type) {
        case Token::Invalid:   return "invalid";

        case Token::Ident:     return "ident";

        case Token::LitInt:    return "int literal";
        case Token::LitFloat:  return "float literal";
        case Token::LitString: return "string literal";
        case Token::True:      return "true";
        case Token::False:     return "false";

        case Token::Func:      return "func";
        case Token::If:        return "if";
        case Token::Else:      return "else";
        case Token::For:       return "for";
        case Token::While:     return "while";
        case Token::Loop:      return "loop";
        case Token::Var:       return "var";
        case Token::Break:     return "break";
        case Token::Continue:  return "continue";
        case Token::Return:    return "return";

        case Token::Void:      return "void";

        case Token::Bool:      return "bool";

        case Token::I8:        return "i8";
        case Token::I16:       return "i16";
        case Token::I32:       return "i32";
        case Token::I64:       return "i64";

        case Token::U8:        return "u8";
        case Token::U16:       return "u16";
        case Token::U32:       return "u32";
        case Token::U64:       return "u64";

        case Token::F32:       return "f32";
        case Token::F64:       return "f64";

        case Token::Char:      return "char";
        case Token::String:    return "string";

        case Token::LParen:    return "(";
        case Token::RParen:    return ")";
        case Token::LBrace:    return "{";
        case Token::RBrace:    return "}";
        case Token::LBracket:  return "[";
        case Token::RBracket:  return "]";

        case Token::Semi:      return ";";
        case Token::Colon:     return ":";
        case Token::Scope:     return "::";
        case Token::Comma:     return ",";
        case Token::Dot:       return ".";
        case Token::RArrow:    return "->";

        case Token::Not:        return "!";
        case Token::BitNot:     return "~";
        case Token::PlusPlus:   return "++";
        case Token::MinusMinus: return "--";

        case Token::Eq:         return "==";
        case Token::NotEq:      return "!=";
        case Token::Greater:    return ">";
        case Token::GreaterEq:  return ">=";
        case Token::Lesser:     return "<";
        case Token::LesserEq:   return "<=";

        case Token::Plus:      return "+";
        case Token::Minus:     return "-";
        case Token::Star:      return "*";
        case Token::Slash:     return "/";
        case Token::Percent:   return "%";
        case Token::LogicAnd:  return "&&";
        case Token::LogicOr:   return "||";
        case Token::BitAnd:    return "&";
        case Token::BitOr:     return "|";
        case Token::BitXOr:    return "^";

        case Token::Assign:    return "=";

        case Token::EndOfFile: return "EOF";

        default:
            SCAR_BUG("missing string for Token::Type {}", (uint16_t)type);
            return "unknown";
        }
    }
    static std::string AsString(const Token& token) {
        return FMT("{}: {}", AsString(token.Span), AsString(token.Type));
    }

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