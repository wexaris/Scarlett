#include "logger.hpp"
#include "cmd_args/args.hpp"
#include "util/logger_ex.hpp"
#include "util/early_exit.hpp"
#include "settings.hpp"

namespace scar {
    namespace log {

        Logger::Logger(std::string_view name, sink_ptr_t sink) :
            sinks({ std::move(sink) }),
            name(name)
        {}

        Logger::Logger(std::string_view name, sink_list_t sinks) :
            sinks(sinks.begin(), sinks.end()),
            name(name)
        {}

        void Logger::throw_if_critical(LogLevel lvl) {
            if (lvl >= Critical) {
                throw FatalError(999);
            }
        }

        void Logger::internal_err(std::string_view msg) const {
            fmt::print(stderr, "[{}] {}\n", this->name, msg);
        }

        LogLevel Logger::update_log_level(LogLevel lvl) {
            return
                (lvl == Debug && !SCAR_DEBUG_ENABLED) ? Off :
                (lvl == Warning && options.warn_as_err) ? Error :
                (lvl == Warning && options.no_warn) ? Off : lvl;
        }

        bool Logger::should_log(LogLevel lvl) const {
            return lvl == Off ? false :
                (lvl == Warning && !options.no_warn) ? false : true;
        }

        void Logger::apply_options(const args::ParsedArgs& args) {
            if (args.args.contains("-Woff")) {
                options.no_warn = true;
            }
            if (args.args.contains("-Werr")) {
                options.warn_as_err = true;
            }
        }

        void Logger::sink_it(std::string_view msg) {
            for (auto& sink : sinks) {
                sink->log(msg);
            }
            flush();
        }

        void Logger::flush() {
            try {
                flush_();
            }
            SCAR_LOGGER_CATCH_EXCEPTION
        }

        void Logger::flush_() {
            for (auto& sink : sinks) {
                sink->flush();
            }
        }

        void Logger::set_formatter(const fmt_ptr_t& fmt) {
            std::lock_guard lock(mutex);
            formatter = fmt;
        }

    }
}