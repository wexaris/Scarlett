#include "errors.hpp"
#include "parse/lex/token.hpp"
#include "files/codepoint.hpp"
#include "common.hpp"

namespace CompileError {

    Base::Base(std::string msg) noexcept :
        std::runtime_error(msg)
    {
        std::cerr << msg << std::endl;
    }

    Generic::Generic(std::string msg) noexcept :
        Base(FMT("Error: " << msg)) {}

    Generic::Generic(const Span& sp, std::string msg) noexcept :
        Base(FMT(sp << ": Error: " << msg)) {}


    Bug::Bug(std::string msg) noexcept :
        Base(FMT("Internal Error: " << msg)) {}

    Bug::Bug(const Span& sp, std::string msg) noexcept :
        Base(FMT(sp << ": Internal Error: " << msg)) {}


    Todo::Todo(std::string msg) noexcept :
        Base(FMT("TODO: " << msg)) {}

    Todo::Todo(const Span& sp, std::string msg) noexcept :
        Base(FMT(sp << ": TODO: " << msg)) {}
}

namespace LexError {

    UnexpectedEOF::UnexpectedEOF(const Span& sp) noexcept :
        Generic(sp, "Unexpected end of file") {}

    UnexpectedSymbol::UnexpectedSymbol(const Span& sp, Codepoint c) noexcept :
        Generic(sp, FMT("Unexpected symbol '" << c << "'")) {}
}

namespace ParseError {

    std::stringstream multi_expected_err(const Span& sp, const Token& tok, std::vector<TokenType> exp) {
        assert(exp.size() > 0);

        auto err = std::stringstream() << sp << ": Unexpected " << tok << ", expected " << token_type::to_string(exp[0]);

        for (int i = 0; i < exp.size() - 1; i++) {
            err << ", " << token_type::to_string(exp[i]);
        }
        err << "or " << token_type::to_string(exp.back());

        return err;
    }

    Unexpected::Unexpected(const Span& sp, const Token& tok) :
        Generic(FMT(sp << ": Unexpected " << tok)) {}

    Unexpected::Unexpected(const Span& sp, const Token& tok, TokenType exp) :
        Generic(FMT(sp << ": Unexpected " << tok << ", expected " << exp)) {}

    Unexpected::Unexpected(const Span& sp, const Token& tok, std::vector<TokenType> exp) :
        Generic(multi_expected_err(sp, tok, exp).str()) {}
}