#pragma once
#include "lex/lexer.hpp"

class Parser {

    Lexer lexer;

public:
    explicit Parser(const std::string& filepath);

    void parse();
};