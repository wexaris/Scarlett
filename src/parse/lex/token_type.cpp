#include "token_type.hpp"
#include "errors/compile_err.hpp"
#include "common.hpp"

std::ostream& operator<<(std::ostream& os, const TokenType& ty) {
    os << token_type::as_str(ty);
    return os;
}

namespace token_type {

    std::string as_str(TokenType ty) {
        switch (ty)
        {
        case END:           return "END";

        case Ident:             return "ident";
        case Lifetime:          return "lifetime";

        
        case LitString:     return "lit_string";
        case LitChar:       return "lit_char";
        case LitInteger:    return "lit_integer";
        case LitFloat:      return "lit_float";
        case True:          return "true";
        case False:         return "false";


        case Var:               return "var";
        case Fun:               return "fun";
        case Struct:            return "struct";
        case Enum:              return "enum";
        case Union:             return "union";
        case Trait:             return "trait";
        case Macro:             return "macro";

        case If:                return "if";
        case Else:              return "else";
        case Loop:              return "loop";
        case For:               return "for";
        case Do:                return "do";
        case While:             return "while";
        case Switch:            return "switch";
        case Case:              return "case";
        case Match:             return "match";
        case Continue:          return "continue";
        case Break:             return "break";
        case Return:            return "return";

        case Pub:               return "pub";
        case Priv:              return "priv";
        case Static:            return "static";
        case Const:             return "const";
        case Mut:               return "mut";


        case Str:           return "str";
        case Char:          return "char";
        case Bool:          return "bool";

        case Isize:         return "isize";
        case I8:            return "i8";
        case I16:           return "i16";
        case I32:           return "i32";
        case I64:           return "i64";

        case Usize:         return "usize";
        case U8:            return "u8";
        case U16:           return "u16";
        case U32:           return "u32";
        case U64:           return "u64";

        case F32:           return "f32";
        case F64:           return "f64";


        case Meh:               return "_";

        case Semi:              return ";";
        case Col:               return ":";
        case Scope:             return "::";
        case Comma:             return ",";
        case Dot:               return ".";
        case DotDot:            return "..";
        case DotDotDot:         return "...";

        case Lparen:            return "(";
        case Rparen:            return ")";
        case Lbrack:            return "[";
        case Rbrack:            return "]";
        case Lbrace:            return "{";
        case Rbrace:            return "}";


        case Not:           return "!";

        case Eq:            return "==";
        case NotEq:         return "!=";
        case Lesser:        return "<";
        case Greater:       return ">";

        case Plus:          return "+";
        case Minus:         return "-";
        case Star:          return "*";
        case Slash:         return "/";
        case Percent:       return "%";
        case LogicAnd:      return "&&";
        case LogicOr:       return "||";
        case And:           return "&";
        case Or:            return "|";
        case Caret:         return "^";
        case Shl:           return "<<";
        case Shr:           return ">>";

        case Assign:        return "=";
        case PlusAssign:    return "+=";
        case MinusAssign:   return "-=";
        case MulAssign:     return "*=";
        case DivAssign:     return "/=";
        case ModAssign:     return "%=";
        case AndAssign:     return "&=";
        case OrAssign:      return "|=";
        case XorAssign:     return "^=";
        case ShlAssign:     return "<<=";
        case ShrAssign:     return ">>=";

        default:
            auto err = FMT("type " << (int)ty << " missing entry in token_type::as_str(TokenType)");
            throw CompileError::Bug(err);
        }
    }
}