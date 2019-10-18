#pragma once
#include "parse/token.hpp"
#include "log/log_builder.hpp"
#include "early_exit.hpp"

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

		// Halt the compilation process and escape.
		using FatalError = early::FatalError;


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

		// Utility class used to recover after compilation errors.
        struct RecoveryUnwind : public std::exception {};

        class ParseError : public ErrorBase<RecoveryUnwind> {

        private:
            virtual void emit_() const override;

            ParseError(log::Level lvl, std::string msg);

        public:
            log::Level lvl;
            std::string msg;

            template<typename... Args>
            static constexpr err::ParseError make(log::Level lvl, Args&&... args) {
                return err::ParseError(lvl, FMT(args...));
            }
        };

    }
}