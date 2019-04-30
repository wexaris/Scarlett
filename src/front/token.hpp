#pragma once
#include "token_type.hpp"
#include "span.hpp"
#include "ast.hpp"

namespace scar {

    struct Token {

        using int_t    = size_t;
        using float_t  = double;
        using str_t    = ast::Name;

        TokenType type;
        Span span;

        bool valid      = true;
        int_t val_i     = 0;
        float_t val_f   = 0;
        str_t val_s     = { 0 };

        source::file_ptr_t::weak_type file = span.file;
        TextPos& lo = span.lo;
        TextPos& hi = span.hi;

        Token() = default;
        Token(TokenType ty, Span sp, bool valid = true);
        Token(TokenType ty, Span sp, int_t v);
        Token(TokenType ty, Span sp, float_t v);
        Token(TokenType ty, Span sp, str_t v);

        inline void operator=(const Token& other) {
            type = other.type;
            span = other.span;
            val_i = other.val_i;
            val_f = other.val_f;
            val_s = other.val_s;
        }

        inline bool is_valid() const    { return valid; }
        inline bool is_eof() const      { return type == END; }
    };

}

template<>
struct fmt::formatter<scar::Token> {

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::Token& tok, FormatContext& ctx) {
        auto type_str = scar::ttype::to_str(tok.type);
        return fmt::format_to(ctx.out(), "{}    {}", type_str, tok.span);
    }
};