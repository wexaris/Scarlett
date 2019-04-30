#pragma once
#include "lex/lexer.hpp"

namespace scar {

    class Parser {

    private:
        Parser(const Parser&) = delete;

        void operator=(const Parser&) = delete;

    public:
        Parser() = default;
    };

}