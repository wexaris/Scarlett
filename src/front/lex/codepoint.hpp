#pragma once
#include "util/range.hpp"
#include "fmt/format.h"
#include <iostream>

namespace scar {

    struct Codepoint {
        uint32_t val;

        constexpr Codepoint(uint32_t x = 0) : val(x) {}
        constexpr Codepoint(const Codepoint& other) : val(other.val) {}

        std::string to_str() const;

        inline bool is_ws() const  { return range::is_whitespace(val); }
        inline bool is_bin() const { return range::is_bin(val); }
        inline bool is_oct() const { return range::is_oct(val); }
        inline bool is_dec() const { return range::is_dec(val); }
        inline bool is_hex() const { return range::is_hex(val); }
        inline bool is_eof() const { return val == '\0'; }

        inline Codepoint& operator=(const Codepoint& other) {
            val = other.val;
            return *this;
        }

        inline bool operator==(Codepoint x) const { return val == x.val; }
        inline bool operator!=(Codepoint x) const { return val != x.val; }
        inline bool operator==(char x) const { return val == static_cast<uint32_t>(x); }
        inline bool operator!=(char x) const { return val != static_cast<uint32_t>(x); }

        std::ostream& operator<<(std::ostream& os) {
            return os << to_str();
        }
    };

    inline std::string& operator+=(std::string& s, Codepoint c) {
        return s += c.to_str();
    }

}

template<>
struct fmt::formatter<scar::Codepoint> {

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::Codepoint& c, FormatContext& ctx) {
        auto cp_str = c.to_str();
        if (cp_str == "{") {
            cp_str = "{{";
        }
        else if (cp_str == "}") {
            cp_str = "}}";
        }
        return fmt::format_to(ctx.out(), cp_str);
    }
};