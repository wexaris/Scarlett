#pragma once
#include "front/token.hpp"
#include "log/logging.hpp"

namespace scar {
    namespace err {

        struct Error : public std::exception {
            log::LogLevel lvl;
            std::string msg;

            Error(log::LogLevel lvl, std::string msg);
            virtual ~Error() = default;

            [[noreturn]] virtual void emit() const;
            inline virtual const char* what() const noexcept override { return msg.c_str(); }
        };

        template<typename E, typename... Args, typename std::enable_if<
            std::is_base_of<Error, E>::value>::type* = nullptr>
        constexpr E make(log::LogLevel lvl, Args&&... args) {
            return E(lvl, fmt::format(args...));
        }

        template<typename... Args>
        constexpr Error make(log::LogLevel lvl, Args&&... args) {
            return Error(lvl, fmt::format(args...));
        }

    }
}