#pragma once
#include "files/utf_reader.hpp"
#include "span.hpp"

class UTFBumpReader {

    UTFReader reader;

    Codepoint curr_cp;
    Codepoint next_cp;

    TextPos text_pos;

    uint last_index = 0;

    /* Get the absolute index of the current token.
    Removes the length of the 
    The reader's 'get_index()' won't be correct, since it has already read the next codepoinh.
    And we can't just -1 because a codepoint can be longer than one char. */
    inline uint get_index() {
        auto curr_idx = last_index;
        last_index = reader.get_index();
        return curr_idx;
    }

public:
    UTFBumpReader(const std::string& filepath);
    ~UTFBumpReader() {}

    void bump(uint n = 1);

    inline Codepoint curr()         { return curr_cp; }
    inline Codepoint next()         { return next_cp; }

    inline TextPos get_pos() const  { return text_pos; }

    inline const SourceFile* get_sf() const { return reader.get_sf(); }
    inline void free_sf()                   { reader.free_sf(); }

    inline bool is_eof() const      { return curr_cp.is_eof(); }
};