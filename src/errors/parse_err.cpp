#include "parse_err.hpp"
#include "parse/lex/token.hpp"

using namespace ParseError;

std::stringstream multi_expected_err(const Span& sp, const Token& tok, std::vector<TokenType> exp);

Unexpected::Unexpected(const Span& sp, const Token& tok) :
    Generic(FMT(sp << ": Unexpected " << tok))
{}

Unexpected::Unexpected(const Span& sp, const Token& tok, TokenType exp) :
    Generic(FMT(sp << ": Unexpected " << tok << ", exprected " << exp))
{}

Unexpected::Unexpected(const Span& sp, const Token& tok, std::vector<TokenType> exp) :
    Generic(multi_expected_err(sp, tok, exp).str())
{}


std::stringstream multi_expected_err(const Span& sp, const Token& tok, std::vector<TokenType> exp) {
    assert(exp.size() > 0);

    std::stringstream err;
    err << sp << ": Unexpected " << tok << ", expected " << token_type::as_str(exp[0]);

    for (int i = 0; i < exp.size(); i++) {
        err << ", ";
        if (i == exp.size() - 1)
            err << "or ";
        err << token_type::as_str(exp[i]);
    }

    return err;
}