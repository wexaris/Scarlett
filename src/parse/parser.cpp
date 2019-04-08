#include "parser.hpp"
#include "errors/errors.hpp"

Parser::Parser(const std::string& filepath) :
    lexer(filepath)
{}

void Parser::parse() {

    throw ParseError::Todo("Add parsing");
}