#pragma once
#include "front/files/source_map.hpp"
#include "front/span.hpp"
#include "codepoint.hpp"

namespace scar {

    class UTFReader {

    private:
        source::file_ptr_t sourcefile;
        size_t next_index = 0;
        bool eof = false;

        TextPos curr_pos;

        Codepoint curr_cp;
        Codepoint next_cp;

        char read_byte();
        Codepoint read();

    public:
        explicit UTFReader(std::string_view path);

        void bump(unsigned int n = 1);

        inline Codepoint curr() const { return curr_cp; }
        inline Codepoint next() const { return next_cp; }

        inline bool is_eof() const                  { return eof; }
        inline TextPos get_pos() const              { return curr_pos; }
        inline size_t get_index() const             { return curr_pos.idx; }
        inline source::file_ptr_t get_sf() const    { return sourcefile; }
    };

}