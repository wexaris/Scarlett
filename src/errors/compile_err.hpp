#pragma once
#include <string>
#include <stdexcept>

struct Span;

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