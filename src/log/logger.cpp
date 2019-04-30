#include "logger.hpp"
#include "cmd_args/args.hpp"

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

        void Logger::apply_options(const args::ParsedArgs& args) {
            if (args.args.contains("-Woff")) {
                options.no_warn = true;
            }
            if (args.args.contains("-Werr")) {
                options.warn_as_err = true;
            }
        }

        void Logger::sink_it(Log& msg) {
            for (auto& sink : sinks) {
                if (msg.level >= Error) {
                    increase_err_count();
                }
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

        void Logger::set_formatter(formatter_ptr_t fmt) {
            for (auto& sink : sinks) {
                sink->set_formatter(fmt);
            }
        }

    }
}