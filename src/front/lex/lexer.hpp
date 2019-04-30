#pragma once
#include "utf_reader.hpp"
#include "front/interner.hpp"
#include "front/token.hpp"
#include <string>

namespace scar {

    class Lexer {

    private:
        UTFReader reader;

        TextPos tok_start_pos;

        inline void bump(unsigned int n = 1) { reader.bump(n); }
        inline Codepoint curr() const { return reader.curr(); }
        inline Codepoint next() const { return reader.next(); }

        void skip_ws_and_comments();

        /* Main tokenization function. */
        Token tokenize_next();

        Token lex_number();

        /* Read and calculate a number in some base. */
        //uint read_numbers(uint base);
        void read_numbers(unsigned int base);
        /* Read and calculate a fraction.
        Base only used in case of errors. */
        //double read_fraction(uint base, const TextPos& start_pos);
        bool read_fraction(unsigned int base);
        /* Read and calculate an exponent.
        Base only used in case of errors. */
        //double read_exponent(uint base, const TextPos& start_pos);
        bool read_exponent(unsigned int base);

        /* Read a hexadecimal escape symbol with some given length. */
        Codepoint read_hex_escape(unsigned int len, Codepoint delim);

        Lexer(const Lexer&) = delete;

        void operator=(const Lexer&) = delete;


    public:
        Lexer(std::string_view in) : reader(in) {}

        Token next_token();

        /* Returns the sourcefile this Lexer is tokenizing. */
        inline const source::file_ptr_t sf() const  { return reader.get_sf(); }
        inline TextPos curr_pos() const             { return reader.get_pos(); }

        /* Returns the current token's span.
        Starts from the stored token start position. */
        inline Span curr_span() const {
            return Span(sf(), tok_start_pos, curr_pos());
        }

        /* Returns a string that starts at the given position and goes on until the current one.
        The start position is inclusive. */
        inline std::string str_from(const TextPos& start) const {
            return sf()->read(start.idx, curr_pos().idx);
        }

        /* Returns the current token's string.
        Starts from the stored token start position. */
        inline std::string curr_str() const {
            return str_from(tok_start_pos);
        }

        /* Interns the current token's string and returns its id.
        Starts from the stored token start position. */
        inline ast::Name curr_name() const {
            return Interner::instance().intern(std::move(curr_str()));
        }
    };

}