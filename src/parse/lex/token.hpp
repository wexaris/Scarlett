#pragma once
#include "token_type.hpp"
#include "span.hpp"
#include <iostream>

struct Token {
    
    TokenType type  = END;
    Span span       = Span();

    Token() = default;
    Token(TokenType t, Span sp);

    inline bool is_eof() const { return type == END; }

    inline const SourceFile* sourcefile() const { return span.source; }
    inline const TextPos& start() const         { return span.lo; }
    inline const TextPos& end() const           { return span.hi; }
};

std::ostream& operator<<(std::ostream& os, const Token& tk);