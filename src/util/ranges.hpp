#pragma once
#include "common.hpp"
#include <optional>

struct Codepoint;

/*	Check if the value is a whitespace. */
constexpr bool is_whitespace(Codepoint c) {
    return
        c.val == ' '    ||
        c.val == '\t'   ||
        c.val == '\r'   ||
        c.val == '\n'   ||
        c.val == 0xC    ||  // ^L
        c.val == 0x85   ||
        c.val == 0x200E ||  // LTR
        c.val == 0x200F ||  // RTL
        c.val == 0x2028 ||  // Line Separator
        c.val == 0x2029;    // Paragrah Separator
}

/* Check if the given integer can be a chracter.  */
constexpr bool is_char(Codepoint i) { return i.val <= CHAR_MAX; }

/* Check if the value is within a certain range of numbers. */
constexpr bool in_range(Codepoint c, uint32_t lo, uint32_t hi) {
    return (lo <= c.val) && (c.val <= hi);
}

/* Check if the value is binary. */
constexpr bool is_bin(Codepoint c)  { return c.val == '0' || c.val == '1'; }
/* Check if the value is octal. */
constexpr bool is_oct(Codepoint c)  { return in_range(c, '0', '7'); }
/* Check if the value is decimal. */
constexpr bool is_dec(Codepoint c)  { return in_range(c, '0', '9'); }
/* Check if the value is hexadecimal. */
constexpr bool is_hex(Codepoint c) {
    return
        in_range(c, '0', '9') ||
        in_range(c, 'a', 'f') ||
        in_range(c, 'A', 'F');
}

/* Check if the value is a character. */
constexpr bool is_alpha(Codepoint c) {
    return in_range(c, 'a', 'z') || in_range(c, 'A', 'Z');
}
/* Check if the value is alphanumeric. */
constexpr bool is_alnum(Codepoint c) { return is_alpha(c) || is_dec(c); }

/* Check if the character can start an identifier. */
constexpr bool is_ident_start(Codepoint c)  { return is_alpha(c) || c == '_'; }
/* Check if the character can continue an identifier. */
constexpr bool is_ident_cont(Codepoint c)   { return is_alnum(c) || c == '_'; }

/* Read character as a number of some base.
If the character isn't in the given base, 'std::nullopt' is retuned. */
constexpr std::optional<uint> get_num(Codepoint c, uint base) {
    assert(base <= 36);

    std::optional<uint> val = std::nullopt;

    if (in_range(c, '0', '9')) {
        val = c.val - '0';
    }
    else if (in_range(c, 'a', 'f')) {
        val = c.val - 'a' + 10;
    }
    else if (in_range(c, 'A', 'F')) {
        val = c.val - 'A' + 10;
    }

    if (val.has_value() && val.value() >= base) {
        val = std::nullopt;
    }

    return val;
}