#pragma once
#include "fmt/format.h"
#include "fmt/core.h"
#include <vector>
#include <string>
#include <sstream>

namespace scar {
    namespace fmt_help {

        template<size_t buff_size>
        static inline std::string_view to_string_view(fmt::basic_memory_buffer<char, buff_size>& buff) {
            return std::string_view(buff.data(), buff.size());
        }

        template<size_t buff_size>
        static inline void append_str(fmt::basic_memory_buffer<char, buff_size>& buff, std::string_view str) {
            auto* str_data = str.data();
            if (str_data != nullptr) {
                buff.append(str_data, str_data + str.size());
            }
        }

        template<typename T, size_t buff_size>
        static inline void append_int(fmt::basic_memory_buffer<char, buff_size> & buff, T x) {
            fmt::format_int i(x);
            buff.append(i.data(), i.data() + i.size());
        }

    }

    namespace col {

        using color_t = std::string_view;

        namespace ansi {
            // Formatting
            static const color_t reset = "\033[m";
            static const color_t bold = "\033[1m";
            static const color_t dark = "\033[2m";
            static const color_t underline = "\033[4m";
            static const color_t blink = "\033[5m";
            static const color_t reverse = "\033[7m";
            static const color_t concealed = "\033[8m";
            static const color_t clear_line = "\033[K";

            // Foreground
            static const color_t fg_black = "\033[30m";
            static const color_t fg_red = "\033[31m";
            static const color_t fg_green = "\033[32m";
            static const color_t fg_yellow = "\033[33m";
            static const color_t fg_blue = "\033[34m";
            static const color_t fg_magenta = "\033[35m";
            static const color_t fg_cyan = "\033[36m";
            static const color_t fg_white = "\033[37m";

            // Background
            static const color_t bg_black = "\033[40m";
            static const color_t bg_red = "\033[41m";
            static const color_t bg_green = "\033[42m";
            static const color_t bg_yellow = "\033[43m";
            static const color_t bg_blue = "\033[44m";
            static const color_t bg_magenta = "\033[45m";
            static const color_t bg_cyan = "\033[46m";
            static const color_t bg_white = "\033[47m";
        }

        static inline std::string with_color(std::vector<color_t> colors, std::string_view txt) {
            std::ostringstream ss;
            for (auto col : colors) {
                ss << col;
            }
            ss << txt << ansi::reset.data();
            return ss.str();
        }

    }
}