#include "codepoint.hpp"
#include "util/ranges.hpp"

bool Codepoint::is_ws() const {
    return is_whitespace(val);
}
bool Codepoint::is_bin() const {
    return ::is_bin(val);
}
bool Codepoint::is_oct() const {
    return ::is_oct(val);
}
bool Codepoint::is_dec() const {
    return ::is_dec(val);
}
bool Codepoint::is_hex() const {
    return ::is_hex(val);
}

bool Codepoint::is_eof() const {
    return val == '\0';
}

std::string& operator+=(std::string& s, const Codepoint& cp) {
    if (cp.val < 128) {
        if (cp.val == '\t')
            s += "\\t";
        else if (cp.val == '\n')
            s += "\\n";
        else if (cp.val == '\r')
            s += "\\r";
        else
            s += (char)cp.val;
    }
    else if (cp.val < (32 << 6)) {
        s += (char)(0xC0 | ((cp.val >> 6) & 0x1F));
        s += (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else if (cp.val < (16 << 12)) {
        s += (char)(0xE0 | ((cp.val >> 12) & 0x0F));
        s += (char)(0x80 | ((cp.val >> 6) & 0x3F));
        s += (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else if (cp.val < (8 << 18)) {
        s += (char)(0xF0 | ((cp.val >> 18) & 0x07));
        s += (char)(0x80 | ((cp.val >> 12) & 0x3F));
        s += (char)(0x80 | ((cp.val >> 6) & 0x3F));
        s += (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else {
        ERR(FMT("Invalid unicode codepoint '" << std::hex << cp.val << "'"));
    }
    return s;
}

std::ostream& operator<<(std::ostream & os, const Codepoint& cp) {
    if (cp.val < 128) {
        if (cp.val == '\t')
            os << "\\t";
        else if (cp.val == '\n')
            os << "\\n";
        else if (cp.val == '\r')
            os << "\\r";
        else
            os << (char)cp.val;
    }
    else if (cp.val < (32 << 6)) {
        os << (char)(0xC0 | ((cp.val >> 6) & 0x1F));
        os << (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else if (cp.val < (16 << 12)) {
        os << (char)(0xE0 | ((cp.val >> 12) & 0x0F));
        os << (char)(0x80 | ((cp.val >> 6) & 0x3F));
        os << (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else if (cp.val < (8 << 18)) {
        os << (char)(0xF0 | ((cp.val >> 18) & 0x07));
        os << (char)(0x80 | ((cp.val >> 12) & 0x3F));
        os << (char)(0x80 | ((cp.val >> 6) & 0x3F));
        os << (char)(0x80 | ((cp.val >> 0) & 0x3F));
    }
    else {
        ERR(FMT("Invalid unicode codepoint '" << std::hex << cp.val << "'"));
    }
    return os;
}