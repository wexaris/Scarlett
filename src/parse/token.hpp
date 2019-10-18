#pragma once
#include "token_type.hpp"
#include "span.hpp"
#include "ast/ast.hpp"

namespace scar {

    struct Token {

        TokenType type;
        Span span;

        bool valid      = true;
        int64_t val_i    = 0;
        double val_f    = 0;
        ast::Name val_s = { 0 };

        Token() = default;
        Token(TokenType ty, Span sp, bool valid = true);
        Token(TokenType ty, Span sp, int64_t v);
        Token(TokenType ty, Span sp, double v);
        Token(TokenType ty, Span sp, ast::Name v);

        inline void operator=(const Token& other) {
            type = other.type;
            span = other.span;
            val_i = other.val_i;
            val_f = other.val_f;
            val_s = other.val_s;
        }

        inline bool operator==(const TokenType& ty) const { return type == ty; }
        inline bool operator!=(const TokenType& ty) const { return type != ty; }

		// Get a string_view of the corresponding source code fragment
        inline std::string_view raw() const {
			return span.file->read(span.idx, span.len);
		}

        inline bool is_valid() const { return valid; }
        inline bool is_eof() const   { return type == END; }
    };

	class TokenStream : public std::vector<Token> {

	private:
		size_t index = 0;

	public:
		TokenStream() = default;

		/* Return the current token */
		inline const Token& get() const { return this->at(index); }

		inline TokenStream& operator++(int) { index++; return *this; }
		inline TokenStream& operator--(int) { index--; return *this; }

		inline bool is_eof() const { return get().is_eof(); }
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