#pragma once
#include "util/interner.hpp"
#include "token_type.hpp"
#include "span.hpp"
#include <variant>
#include <iostream>

struct TokenValue {
    using IntType    = size_t;
    using FloatType  = double;
    using StrType    = ast::Name;

    using DataVariant = std::variant<IntType, FloatType, StrType>;
    DataVariant data;

    TokenValue() = default;
    TokenValue(IntType v) : data(v) {}
    TokenValue(FloatType v) : data(v) {}
    TokenValue(StrType v) : data(v) {}

    inline void set(DataVariant v) { data = v; }

    inline IntType get_i() const   { return std::get<0>(data); }
    inline FloatType get_f() const { return std::get<1>(data); }
    inline StrType get_s() const   { return std::get<2>(data); }
};

struct Token {
    
    TokenType type  = END;
    Span span       = Span();

    TokenValue val  = TokenValue();

    Token() = default;
    Token(TokenType t, Span sp, const TokenValue& val = TokenValue());

    inline bool is_eof() const { return type == END; }

    /* Get the raw string that the token came from.
    If 'rm_quotes' is true, string literals, char literals, and lifetimes
    will have their surrounding quotes removed. */
    std::string_view raw(bool rm_quotes = false) const;

    inline const SourceFile* sourcefile() const { return span.source; }
    inline const TextPos& start() const         { return span.lo; }
    inline const TextPos& end() const           { return span.hi; }
};

std::ostream& operator<<(std::ostream& os, const Token& tk);