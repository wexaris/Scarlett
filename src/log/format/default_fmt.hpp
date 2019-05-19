#pragma once
#include "formatter.hpp"
#include "log/util/format_help.hpp"
#include <unordered_map>

namespace scar {
    namespace log {
        namespace format {

            class DefaultFormatter final : public FormatterBase {

            private:
                struct ColorPair { std::string lvl, msg; };

                std::unordered_map<LogLevel, ColorPair> logcolors;

                std::string end = col::ansi::reset + "\n";

            public:
                DefaultFormatter();
                ~DefaultFormatter() = default;

                // level[code]: message
                //   --> file:line:col
                //    |
                // ln | bad code preview
                //    |     ^^^^
                void fmt(const Log& msg, ::fmt::memory_buffer& buff) const final override;

                inline ColorPair get_lvl_color(LogLevel lvl) const {
                    return logcolors.at(lvl);
                }
                inline void set_lvl_color(LogLevel lvl, col::color_t lvl_col, col::color_t msg_col) {
                    logcolors[lvl] = { lvl_col, msg_col };
                }

                inline std::string_view get_ending() const { return end; }
                inline void set_ending(std::string e) { end = std::move(e); }
            };

        }
    }
}