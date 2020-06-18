#pragma once
#include "Parse/Span.hpp"
#include "Parse/Interner.hpp"
#include <variant>

namespace scar {

    class Token {
    public:
        enum Type : uint16_t {
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

        Token(const Span& span)                          : m_Type(Invalid), m_Span(span) {}
        Token(Type type, const Span& span)               : m_Type(type), m_Span(span) {}
        Token(Type type, Type subType, uint64_t val, const Span& span) :
            m_Type(type), m_Span(span), m_Value(std::in_place_index_t<0>{}, val), m_ValueType(subType) {}
        Token(Type type, Type subType, double val, const Span& span)   :
            m_Type(type), m_Span(span), m_Value(std::in_place_index_t<1>{}, val), m_ValueType(subType) {}
        Token(Type type, std::string_view val, const Span& span) :
            m_Type(type), m_Span(span), m_Value(std::in_place_index_t<2>{}, Interner::Intern(val)), m_ValueType(String) {}

        Type GetType() const { return m_Type; }
        Type GetValueType() const { return m_ValueType; }
        const Span& GetSpan() const { return m_Span; }
        TextPosition GetTextPos() const { return TextPosition(m_Span.Line, m_Span.Col, m_Span.Index); }
        uint64_t GetInt() const {
            if (m_Type != LitInt) { SCAR_BUG("cannot get integer value from non-integer Token!"); }
            return std::get<0>(m_Value);
        }
        double GetFloat() const {
            if (m_Type != LitFloat) { SCAR_BUG("cannot get float value from non-float Token!"); }
            return std::get<1>(m_Value);
        }
        Interner::StringID GetName() const {
            if (m_Type != LitString && m_Type != Ident) { SCAR_BUG("cannot get StringID from non-string Token!"); }
            return std::get<2>(m_Value);
        }
        std::string_view GetString() const { return Interner::GetString(GetName()); }

        bool IsEOF() const { return m_Type == Type::EndOfFile; }
        bool IsValid() const { return m_Type != Type::Invalid; }

        // Get the corresponding source code fragment
        std::string_view GetRaw() const {
            return m_Span.File->GetString(m_Span.Index, m_Span.Length);
        }

        bool operator==(const Token::Type& type) const { return m_Type == type; }
        bool operator!=(const Token::Type& type) const { return m_Type != type; }

    private:
        const Type m_Type = Type::Invalid;
        const Span m_Span;
        const std::variant<uint64_t, double, Interner::StringID> m_Value;
        const Type m_ValueType = Type::Invalid;
    };

    static std::string_view AsString(Token::Type type) {
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
        return FMT("{}: {}", AsString(token.GetSpan()), AsString(token.GetType()));
    }

    static std::ostream& operator<<(std::ostream& os, Token::Type tokenType) {
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