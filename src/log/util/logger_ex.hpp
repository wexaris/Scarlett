#pragma once
#include <stdexcept>
#include "format_help.hpp"

namespace scar {
    namespace log {

        class LoggerException : public std::exception {

        private:
            std::string msg;

        public:
            LoggerException(std::string msg) :
                msg(std::move(msg))
            {}

            inline const char* what() const noexcept override {
                return msg.c_str();
            }
        };

    }
}