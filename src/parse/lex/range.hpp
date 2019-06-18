#pragma once
#include <optional>

namespace scar {
    namespace range {

        /*	Check if the value is a whitespace. */
        constexpr bool is_whitespace(uint32_t x) {
            return
                x == ' '    ||
                x == '\t'   ||
                x == '\r'   ||
                x == '\n'   ||
                x == 0xC    ||  // ^L
                x == 0x85   ||
                x == 0x200E ||  // LTR
                x == 0x200F ||  // RTL
                x == 0x2028 ||  // Line Separator
                x == 0x2029;    // Paragraph Separator
        }

        /* Check if the given integer can be a character.  */
        constexpr bool is_char(uint32_t x) { return x <= 0xffff; }

        /* Check if the value is within a certain range of numbers. */
        constexpr bool in_range(uint32_t x, uint32_t lo, uint32_t hi) {
            return (lo <= x) && (x <= hi);
        }

        /* Check if the value is binary. */
        constexpr bool is_bin(uint32_t x) { return x == '0' || x == '1'; }
        /* Check if the value is octal. */
        constexpr bool is_oct(uint32_t x) { return in_range(x, '0', '7'); }
        /* Check if the value is decimal. */
        constexpr bool is_dec(uint32_t x) { return in_range(x, '0', '9'); }
        /* Check if the value is hexadecimal. */
        constexpr bool is_hex(uint32_t x) {
            return
                in_range(x, '0', '9') ||
                in_range(x, 'a', 'f') ||
                in_range(x, 'A', 'F');
        }

        /* Check if the value is a character. */
        constexpr bool is_alpha(uint32_t x) {
            return in_range(x, 'a', 'z') || in_range(x, 'A', 'Z');
        }
        /* Check if the value is alphanumeric. */
        constexpr bool is_alnum(uint32_t x) { return is_alpha(x) || is_dec(x); }

        /* Check if the character can start an identifier. */
        constexpr bool is_ident_start(uint32_t x) { return is_alpha(x) || x == '_'; }
        /* Check if the character can continue an identifier. */
        constexpr bool is_ident_cont(uint32_t x) { return is_alnum(x) || x == '_'; }

        /* Read character as a number of some base.
        If the character isn't in the given base, 'std::nullopt' is retuned. */
        static inline std::optional<unsigned int> get_num(uint32_t x, unsigned int base) {
            unsigned int val = 0;

            if (in_range(x, '0', '9')) {
                val = x - '0';
            }
            else if (in_range(x, 'a', 'f')) {
                val = x - 'a' + 10;
            }
            else if (in_range(x, 'A', 'F')) {
                val = x - 'A' + 10;
            }
            else {
                return std::nullopt;
            }

            if (val >= base) {
                return std::nullopt;
            }

            return val;
        }

    }
}