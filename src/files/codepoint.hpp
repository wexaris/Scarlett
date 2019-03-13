#pragma once
#include <ostream>

struct Codepoint {
    uint32_t val;

    constexpr Codepoint::Codepoint(uint32_t x = 0) : val(x) {}
    constexpr Codepoint::Codepoint(Codepoint&& other) : Codepoint(other.val) {}
    constexpr Codepoint::Codepoint(const Codepoint& other) : Codepoint(other.val) {}

    bool is_ws() const;
    bool is_bin() const;
    bool is_oct() const;
    bool is_dec() const;
    bool is_hex() const;
    bool is_eof() const;

    inline Codepoint& operator=(const Codepoint& other) {
        val = other.val;
        return *this;
    }

    inline bool operator==(char x) const { return val == static_cast<int32_t>(x); }
    inline bool operator!=(char x) const { return val != static_cast<int32_t>(x); }

    inline bool operator==(Codepoint x) const { return val == x.val; }
    inline bool operator!=(Codepoint x) const { return val != x.val; }
};

std::ostream& operator<<(std::ostream& os, const Codepoint& cp);
std::string& operator+=(std::string& s, const Codepoint& cp);