#include "compile_err.hpp"
#include "span.hpp"
#include "common.hpp"

using namespace CompileError;

Base::Base(std::string msg) noexcept :
    std::runtime_error(msg)
{
    std::cerr << msg << std::endl;
}


Generic::Generic(std::string msg) noexcept :
    Base(FMT("Error: " << msg))
{}

Generic::Generic(const Span& sp, std::string msg) noexcept :
    Base(FMT(sp << ": Error: " << msg))
{}


Bug::Bug(std::string msg) noexcept :
    Base(FMT("Internal Error: " << msg))
{}

Bug::Bug(const Span& sp, std::string msg) noexcept :
    Base(FMT(sp << ": Internal Error: " << msg))
{}


Todo::Todo(std::string msg) noexcept :
    Base(FMT("TODO: " << msg))
{}

Todo::Todo(const Span& sp, std::string msg) noexcept :
    Base(FMT(sp << ": TODO: " << msg))
{}