#pragma once
#include "compile_err.hpp"
#include <vector>

enum TokenType;
struct Token;

namespace ParseError {

    using CompileError::Generic;
    using CompileError::Bug;
    using CompileError::Todo;

    class Unexpected : public CompileError::Generic {

    public:
        Unexpected(const Span& sp, const Token& tok);
        Unexpected(const Span& sp, const Token& tok, TokenType exp);
        Unexpected(const Span& sp, const Token& tok, std::vector<TokenType> exp);
        virtual ~Unexpected() noexcept override {}
    };

}