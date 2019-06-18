#include "codepoint.hpp"
#include "log/logging.hpp"

namespace scar {

    std::string Codepoint::to_str() const {
        std::string s;
        if (val < 128) {
            if (val == '\t')        { s += "\\t"; }
            else if (val == '\n')   { s += "\\n"; }
            else if (val == '\r')   { s += "\\r"; }
            else                    { s += (char)val; }
        }
        else if (val < (32 << 6)) {
            s += (char)(0xC0 | ((val >> 6) & 0x1F));
            s += (char)(0x80 | ((val >> 0) & 0x3F));
        }
        else if (val < (16 << 12)) {
            s += (char)(0xE0 | ((val >> 12) & 0x0F));
            s += (char)(0x80 | ((val >> 6) & 0x3F));
            s += (char)(0x80 | ((val >> 0) & 0x3F));
        }
        else if (val < (8 << 18)) {
            s += (char)(0xF0 | ((val >> 18) & 0x07));
            s += (char)(0x80 | ((val >> 12) & 0x3F));
            s += (char)(0x80 | ((val >> 6) & 0x3F));
            s += (char)(0x80 | ((val >> 0) & 0x3F));
        }
        else {
            log::critical("invalid Unicode codepoint '{:x}'", val);
        }
        return s;
    }

}