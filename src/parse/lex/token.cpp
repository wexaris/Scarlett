#include "token.hpp"

Token::Token(TokenType t, Span sp) :
    type(t),
    span(mv(sp))
{}

std::ostream& operator<<(std::ostream& os, const Token& tk) {
    os << tk.type << " " << tk.span;
    return os;
}