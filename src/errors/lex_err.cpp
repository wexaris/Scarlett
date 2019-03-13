#include "lex_err.hpp"
#include "files/codepoint.hpp"
#include "span.hpp"
#include <iostream>

using namespace LexError;

UnexpectedEOF::UnexpectedEOF(const Span& sp) noexcept :
    Generic(sp, "Unexpected end of file")
{}

UnexpectedSymbol::UnexpectedSymbol(const Span& sp, Codepoint c) noexcept :
    Generic(sp, FMT("Unexpected symbol '" << c << "'"))
{}