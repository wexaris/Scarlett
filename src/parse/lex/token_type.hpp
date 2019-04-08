#pragma once
#include "errors/errors.hpp"
#include "util/magic_enum.hpp"
#include "common.hpp"

enum TokenType {
    END = 0,

    /// Identifiers
    Ident,
    Lifetime,

    /// Literals
    LitString,
    LitChar,
    LitInteger,
    LitFloat,
    True,
    False,

    /// Keywords
    Var,
    Fun,
    Struct,
    Enum,
    Union,
    Trait,
    Macro,

    If,
    Else,
    Loop,
    For,
    Do,
    While,
    Switch,
    Case,
    Match,
    Continue,
    Break,
    Return,

    Pub,
    Priv,
    Static,
    Const,
    Mut,

    /// Primitives
    Str,
    Char,
    Bool,

    Isize,
    I8,
    I16,
    I32,
    I64,

    Usize,
    U8,
    U16,
    U32,
    U64,

    F32,
    F64,

    /// Symbols
    Meh,            // _

    Semi,           // ;
    Col,            // :
    Scope,          // ::
    Comma,          // ,
    Dot,            // .
    DotDot,         // ..
    DotDotDot,      // ...

    Lparen,         // (
    Rparen,         // )
    Lbrack,         // [
    Rbrack,         // ]
    Lbrace,         // {
    Rbrace,         // }

    /// Operators
    Not,            // !

    Eq,             // ==
    NotEq,          // !=
    Greater,        // >
    Lesser,         // <

    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    LogicAnd,       // &&
    LogicOr,        // ||
    And,            // &
    Or,             // |
    Caret,          // ^
    Shl,            // <<
    Shr,            // >>

    Assign,         // =
    PlusAssign,     // +=
    MinusAssign,    // -=
    MulAssign,      // *=
    DivAssign,      // /=
    ModAssign,      // %=
    AndAssign,      // &=
    OrAssign,       // |=
    XorAssign,      // ^=
    ShlAssign,      // <<=
    ShrAssign,      // >>=
};

namespace token_type {

    static std::string to_string(TokenType ty) {
        auto ty_str = magic_enum::enum_name<TokenType>(ty);
        if (!ty_str.has_value()) {
            auto err = FMT("invalid TokenType value '" << (int)ty << "' passed to 'token_type::to_string()'");
            throw CompileError::Bug(err);
        }
        return std::string(ty_str.value());
    }

    static TokenType from_string(std::string str) {
        auto ty_val = magic_enum::enum_cast<TokenType>(str);
        if (!ty_val.has_value()) {
            auto err = FMT("invalid TokenType value '" << str << "' passed to 'token_type::from_string()'");
            throw CompileError::Bug(err);
        }
        return ty_val.value();
    }
}

inline std::ostream& operator<<(std::ostream& os, const TokenType& ty) {
    return os << token_type::to_string(ty);
}