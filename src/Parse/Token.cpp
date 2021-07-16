#include "scarpch.hpp"
#include "Parse/Token.hpp"

namespace scar {

    Token::Token(const TextSpan& span) : Token(Token::Invalid, span) {}

    Token::Token(TokenType type, const TextSpan& span) :
        Type(type), Span(span)
    {}

    Token::Token(TokenType type, TokenType valType, uint64_t val, const TextSpan& span) :
        Type(type), LiteralType(valType), Span(span), m_Value(std::in_place_index_t<0>{}, val)
    {}

    Token::Token(TokenType type, TokenType valType, double val, const TextSpan& span) :
        Type(type), LiteralType(valType), Span(span), m_Value(std::in_place_index_t<1>{}, val)
    {}

    Token::Token(TokenType type, std::string_view val, const TextSpan& span) :
        Type(type), LiteralType(String), Span(span), m_Value(std::in_place_index_t<2>{}, Interner::Intern(val))
    {}

    uint64_t Token::GetInt() const {
        SCAR_ASSERT(Type == LitInt, "trying to get integer value from non-integer token!");
        return std::get<0>(m_Value);
    }

    double Token::GetFloat() const {
        SCAR_ASSERT(Type == LitFloat, "trying to get floating point value from non-float Token!");
        return std::get<1>(m_Value);
    }

    Interner::StringID Token::GetName() const {
        SCAR_ASSERT(Type == LitString || Type == Ident, "trying to get StringID from non-string Token!");
        return std::get<2>(m_Value);
    }

    const std::string& Token::GetString() const {
        return Interner::GetString(GetName());
    }

    std::string_view AsString(Token::TokenType type) {
        switch (type) {
        case Token::Invalid:   return "invalid";

        case Token::Ident:     return "identifier";

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
        case Token::As:        return "as";

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
            return "<unknown>";
        }
    }

    std::string AsString(const Token & token) {
        return FMT("{}: {}", AsString(token.Span), AsString(token.Type));
    }

}