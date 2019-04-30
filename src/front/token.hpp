#pragma once
#include "token_type.hpp"
#include "ast.hpp"
#include "span.hpp"

namespace scar {

    struct Token {

        using int_t    = size_t;
        using float_t  = double;
        using str_t    = ast::Name;

        TokenType type;
        Span span;

        int_t val_i;
        float_t val_f;
        str_t val_s;

        source::file_ptr_t::weak_type file = span.file;
        TextPos& lo = span.lo;
        TextPos& hi = span.hi;

        Token() = default;
        Token(TokenType ty, Span sp) :
            type(ty),
            span(sp)
        {}
        Token(TokenType ty, Span sp, int_t v) : Token(ty, sp) {
            val_i = v;
        }
        Token(TokenType ty, Span sp, float_t v) : Token(ty, sp) {
            val_f = v;
        }
        Token(TokenType ty, Span sp, str_t v) : Token(ty, sp) {
            val_s = v;
        }

        void operator=(const Token& other) {
            type = other.type;
            span = other.span;
            val_i = other.val_i;
            val_f = other.val_f;
            val_s = other.val_s;
        }

        inline bool is_eof() const { return type == END; }
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