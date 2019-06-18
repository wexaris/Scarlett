#pragma once
#include "fmt/format.h"
#include <string>

namespace scar {

    enum TokenType {
        END = 0,

        /// Identifiers
        Ident,
        Lifetime,

        /// Literals
        LitString,
        LitChar,
        LitBool,
        LitInteger,
        LitFloat,

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
        Rarrow,         // ->

        Lparen,         // (
        Rparen,         // )
        Lbrack,         // [
        Rbrack,         // ]
        Lbrace,         // {
        Rbrace,         // }

        /// Operators
        Not,            // !
        BitNot,         // ~
        PlusPlus,       // ++
        MinusMinus,     // --

        Eq,             // ==
        NotEq,          // !=
        Greater,        // >
        GreaterEq,      // >=
        Lesser,         // <
        LesserEq,       // <=

        Plus,           // +
        Minus,          // -
        Star,           // *
        Slash,          // /
        Percent,        // %
        LogicAnd,       // &&
        LogicOr,        // ||
        BitAnd,         // &
        BitOr,          // |
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
        ShrAssign       // >>=
    };

    namespace ttype {
        std::string to_str(TokenType ty);
    }

}

template<>
struct fmt::formatter<scar::TokenType> {

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::TokenType& ty, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), scar::ttype::to_str(ty));
    }
};