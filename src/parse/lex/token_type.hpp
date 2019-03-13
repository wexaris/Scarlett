#pragma once
#include <string>

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

std::ostream& operator<<(std::ostream& os, const TokenType& tk);

namespace token_type {

    std::string as_str(TokenType ty);
}