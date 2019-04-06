#include "token.hpp"
#include "files/source_file.hpp"

Token::Token(TokenType t, Span sp) :
    type(t),
    span(mv(sp))
{}

std::string_view Token::raw() const {
    size_t lo = span.lo.idx;
    size_t hi = span.hi.idx;
    
    if (type == LitString || type == LitChar) {
        lo++;
        hi--;
    }
    else if (type == Lifetime) {
        lo++;
    }
    
    return span.source->get(lo, hi);
}

std::ostream& operator<<(std::ostream& os, const Token& tk) {
    os << tk.span.source->get(tk.span.lo.idx, tk.span.hi.idx) << " " << tk.span;
    return os;
}