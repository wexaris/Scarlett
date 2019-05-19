#pragma once
#include "front/token.hpp"
#include "log/log.hpp"

namespace scar {
    namespace err {

        // Base class for front end errors
        template<class Except, typename std::enable_if<
            std::is_base_of<std::exception, Except>::value>::type* = nullptr>
        class ErrorBase {

        private:
            virtual void emit_() const = 0;

        public:
            constexpr ErrorBase() = default;
            virtual ~ErrorBase() = default;

            [[noreturn]] inline void emit() const {
                emit_();
                throw Except();
            }
        };


        struct RecoveryUnwind : public std::exception {};

        class ParseError : public ErrorBase<RecoveryUnwind> {

        private:
            virtual void emit_() const override;

        public:
            log::LogLevel lvl;
            std::string msg;

            ParseError(log::LogLevel lvl, std::string msg);

            template<typename... Args>
            static constexpr err::ParseError make(log::LogLevel lvl, Args&& ... args) {
                return err::ParseError(lvl, fmt::format(args...));
            }
        };

    }
}