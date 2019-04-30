#include "utf_reader.hpp"
#include "log/logging.hpp"
#include "settings.hpp"
#include <string.h>

namespace scar {

    UTFReader::UTFReader(std::string_view path) :
        sourcefile(source::SourceMap::instace().load(path))
    {
        next_cp = read();
        curr_cp = read();
    }

    void UTFReader::bump(unsigned int n) {
        for (; n > 0; n--) {
            if (curr_cp == '\n') {
                curr_pos.line++;
                curr_pos.col = 1;
            }
            else {
                curr_pos.col++;
            }

            curr_cp = next_cp;

            next_cp = read();
        }
    }

    inline void UTFReader::fill_buffer() {
        buffer = sourcefile->read(next_index, BUFFER_READ_SIZE);
    }

    char UTFReader::read_byte() {
        if (next_index % (BUFFER_READ_SIZE - 1) == 0) {
            fill_buffer();
        }

        char c = buffer[next_index];
        if (c == '\0') {
            eof = true;
        }
        else {
            next_index++;
        }

        return c;
    }

    Codepoint UTFReader::read() {
        curr_pos.idx = next_index;

        uint8_t v1 = read_byte();

        if (v1 < 128) {
            return Codepoint(v1);
        }
        else if ((v1 & 0xC0) == 0x80) {
            // Invalid
            return Codepoint(0xFFFE);
        }
        else if ((v1 & 0xE0) == 0xC0) {
            // Two bytes
            uint8_t e1 = read_byte();
            if ((e1 & 0xC0) != 0x80)  return { 0xFFFE };

            uint32_t outval =
                ((v1 & 0x1F) << 6) |
                ((e1 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else if ((v1 & 0xF0) == 0xE0) {
            // Three bytes
            uint8_t e1 = read_byte();
            if ((e1 & 0xC0) != 0x80)  return { 0xFFFE };
            uint8_t e2 = read_byte();
            if ((e2 & 0xC0) != 0x80)  return { 0xFFFE };

            uint32_t outval =
                ((v1 & 0x0F) << 12) |
                ((e1 & 0x3F) << 6)  |
                ((e2 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else if ((v1 & 0xF8) == 0xF0) {
            // Four bytes
            uint8_t e1 = read_byte();
            if ((e1 & 0xC0) != 0x80)  return { 0xFFFE };
            uint8_t e2 = read_byte();
            if ((e2 & 0xC0) != 0x80)  return { 0xFFFE };
            uint8_t e3 = read_byte();
            if ((e3 & 0xC0) != 0x80)  return { 0xFFFE };

            uint32_t outval =
                ((v1 & 0x07) << 18) |
                ((e1 & 0x3F) << 12) |
                ((e2 & 0x3F) << 6)  |
                ((e3 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else {
            log::get_default()->critical("invalid UTF-8; code is too long");
            return Codepoint(0); // hide 'reaches end of non-void function' warning
        }
    }

}