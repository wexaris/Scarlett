#include "utf_reader.hpp"
#include "source_map.hpp"
#include "errors/lex_err.hpp"

UTFReader::UTFReader(const std::string& filepath) :
    sourcefile(SourceMap::request(filepath))
{}

Codepoint UTFReader::read() {
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
            ((e1 & 0x3F) << 6) |
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

        uint32_t outval
            = ((v1 & 0x07) << 18)
            | ((e1 & 0x3F) << 12)
            | ((e2 & 0x3F) << 6)
            | ((e3 & 0x3F) << 0)
            ;
        return Codepoint(outval);
    }
    else {
        throw CompileError::Generic("Invalid UTF-8 (too long)");
    }
}

char UTFReader::read_byte() {
    char c = (*sourcefile)[index];
    
    if (c != '\0') {
        index++;
    }

    return c;
}