#pragma once
#include "compile_err.hpp"

struct Span;
struct Codepoint;

namespace LexError {

    using CompileError::Generic;
    using CompileError::Bug;
    using CompileError::Todo;

    class UnexpectedEOF : public CompileError::Generic {

    public:
        UnexpectedEOF(const Span& sp) noexcept;
        virtual ~UnexpectedEOF() noexcept override {}
    };

    class UnexpectedSymbol : public CompileError::Generic {

    public:
        UnexpectedSymbol(const Span& sp, Codepoint c) noexcept;
        virtual ~UnexpectedSymbol() noexcept override {}
    };
}