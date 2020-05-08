#include "scarpch.hpp"
#include "Parse/Lex/Codepoint.hpp"

namespace scar {

    std::string AsString(const Codepoint& cp) {
        std::string s;
        if (cp.Value < 128) {
            if (cp.Value == '\t')      { s = "\\t"; }
            else if (cp.Value == '\n') { s = "\\n"; }
            else if (cp.Value == '\r') { s = "\\r"; }
            else                       { s = (char)cp.Value; }
        }
        else if (cp.Value < (32 << 6)) {
            s += (char)(0xC0 | ((cp.Value >> 6) & 0x1F));
            s += (char)(0x80 | ((cp.Value >> 0) & 0x3F));
        }
        else if (cp.Value < (16 << 12)) {
            s += (char)(0xE0 | ((cp.Value >> 12) & 0x0F));
            s += (char)(0x80 | ((cp.Value >> 6) & 0x3F));
            s += (char)(0x80 | ((cp.Value >> 0) & 0x3F));
        }
        else if (cp.Value < (8 << 18)) {
            s += (char)(0xF0 | ((cp.Value >> 18) & 0x07));
            s += (char)(0x80 | ((cp.Value >> 12) & 0x3F));
            s += (char)(0x80 | ((cp.Value >> 6) & 0x3F));
            s += (char)(0x80 | ((cp.Value >> 0) & 0x3F));
        }
        else {
            throw RogueError(ErrorCode::InvalidUTF8, FMT("invalid UTF-8; value is too large: {0:#X}", cp.Value));
        }
        return s;
    }

}