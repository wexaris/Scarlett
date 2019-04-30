#pragma once
#include <stdexcept>
#include <string>

namespace scar {

    class EarlyExit final : public std::exception {

    public:
        const int code;

        EarlyExit(int code) : code(code) {}
        ~EarlyExit() = default;

        inline const char* what() const noexcept override {
            return "";
        }
    };

    class FatalError : public std::exception {

    public:
        const int code;

        FatalError(int code) : code(code) {}
        ~FatalError() = default;

        inline const char* what() const noexcept override {
            return "";
        }
    };

}