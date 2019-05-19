#include "default_fmt.hpp"

namespace scar {
    namespace log {
        namespace format {

            DefaultFormatter::DefaultFormatter() {
                logcolors[Off] = { col::ansi::reset, col::ansi::reset };
                logcolors[Trace] = { col::ansi::reset, col::ansi::reset };
                logcolors[Debug] = { col::ansi::reset, col::ansi::reset };
                logcolors[Info] = { col::ansi::reset, col::ansi::reset };
                logcolors[Warning] = { col::ansi::fg_yellow + col::ansi::bold, col::ansi::fg_yellow };
                logcolors[Error] = { col::ansi::fg_red + col::ansi::bold, col::ansi::fg_red };
                logcolors[Critical] = { col::ansi::bold + col::ansi::bg_red, col::ansi::bg_red };
            }

            void DefaultFormatter::fmt(const Log& msg, ::fmt::memory_buffer& buff) const {
                // Add log level
                if (msg.level >= Warning) {
                    auto lvl_txt = loglevels.at(msg.level);
                    fmt_help::append_str(buff, col::with_color(logcolors.at(msg.level).lvl, lvl_txt + ": "));
                }
                fmt_help::append_str(buff, col::with_color(logcolors.at(msg.level).msg, msg.payload));

                fmt_help::append_str(buff, end);
            }

        }
    }
}