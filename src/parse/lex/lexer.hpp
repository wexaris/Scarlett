#pragma once
#include "bump_reader.hpp"
#include "token.hpp"
#include "common.hpp"

class Lexer {

    UTFBumpReader reader;

    Codepoint curr_c;
    Codepoint next_c;

    Token tokenize_next(const TextPos& start_pos);

    /* Read and calculate a number in some base. */
    uint read_numbers(uint base);
    /* Read and calculate a fraction.
    Base only used in case of errors. */
    double read_fraction(uint base, const TextPos& start_pos);
    /* Read and calculate an exponent.
    Base only used in case of errors. */
    double read_exponent(uint base, const TextPos& start_pos);

    Token lex_number(const TextPos& start_pos);

    /* Reads a hexadecimal escape symbol with some given length. */
    Codepoint read_hex_escape(uint len, Codepoint delim, const TextPos& start_pos);

public:
    explicit Lexer(const std::string& filepath);

    void bump(uint n = 1);

    Token next_token();

    inline Codepoint curr_char() const { return curr_c; }
    inline Codepoint next_char() const { return next_c; }

    inline TextPos get_pos() const          { return reader.get_pos(); }
    inline const SourceFile* get_sf() const { return reader.get_sf(); }

    inline void free_sf() { reader.free_sf(); }
};