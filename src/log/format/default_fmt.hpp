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
                DefaultFormatter() {
                    logcolors[Off] = { col::ansi::reset, col::ansi::reset };
                    logcolors[Trace] = { col::ansi::reset, col::ansi::reset };
                    logcolors[Debug] = { col::ansi::reset, col::ansi::reset };
                    logcolors[Info] = { col::ansi::reset, col::ansi::reset };
                    logcolors[Warning] = { col::ansi::fg_yellow + col::ansi::bold, col::ansi::fg_yellow };
                    logcolors[Error] = { col::ansi::fg_red + col::ansi::bold, col::ansi::fg_red };
                    logcolors[Critical] = { col::ansi::bold + col::ansi::bg_red, col::ansi::bg_red };
                }
                ~DefaultFormatter() = default;

                // level[code]: message
                //   --> file:line:col
                //    |
                // ln | bad code preview
                //    |     ^^^^
                void fmt(const Log& msg, ::fmt::memory_buffer& buff) const final override {
                    // Add log level
                    if (msg.level >= Warning) {
                        auto lvl_txt = loglevels.at(msg.level);
                        fmt_help::append_str(buff, col::with_color(logcolors.at(msg.level).lvl, lvl_txt + ": "));
                    }
                    fmt_help::append_str(buff, col::with_color(logcolors.at(msg.level).msg, msg.payload));

                    fmt_help::append_str(buff, end);
                }

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