#pragma once
#include <stdexcept>
#include <vector>
#include <string>

struct Span;
struct Token;
struct Codepoint;
enum TokenType;

namespace CompileError {

    class Base : public std::runtime_error {
    public:
        Base(std::string msg) noexcept;
        virtual ~Base() noexcept {}
    };

    class Generic : public Base {
    public:
        Generic(std::string msg) noexcept;
        Generic(const Span& sp, std::string msg) noexcept;
        virtual ~Generic() noexcept override {}
    };

    class Bug : public Base {
    public:
        Bug(std::string msg) noexcept;
        Bug(const Span& sp, std::string msg) noexcept;
        virtual ~Bug() noexcept override {}
    };

    class Todo : public Base {
    public:
        Todo(std::string msg) noexcept;
        Todo(const Span& sp, std::string msg) noexcept;
        virtual ~Todo() noexcept override {}
    };
}

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