#pragma once
#include "sinks/sink.hpp"
#include <functional>
#include <memory>
#include <mutex>

namespace scar {
    namespace args { struct ParsedArgs; }

    namespace log {

#define SCAR_LOGGER_CATCH_EXCEPTION catch (const std::exception& e) { internal_err(e.what()); }

        using fmt_ptr_t = std::shared_ptr<format::FormatterBase>;
        using sink_ptr_t = std::shared_ptr<sinks::SinkBase>;
        using sink_list_t = std::initializer_list<sink_ptr_t>;
        using err_handler_t = std::function<void(std::string_view msg)>;

        class Logger {

        private:
            std::vector<sink_ptr_t> sinks;
            fmt_ptr_t formatter = std::make_shared<format::DefaultFormatter>();
            std::mutex mutex;

            virtual void sink_it(std::string_view msg);
            virtual void flush_();

            void throw_if_critical(LogLevel lvl);

            void internal_err(std::string_view msg) const;

        public:
            const std::string_view name;

            struct Options {
                bool no_warn;
                bool warn_as_err;
            } options;

            Logger(std::string_view name, sink_ptr_t sink);
            Logger(std::string_view name, sink_list_t sinks);
            virtual ~Logger() = default;

            void flush();

            LogLevel update_log_level(LogLevel lvl);
            bool should_log(LogLevel lvl) const;

            template<typename... Args>
            inline void log(LogLevel lvl, const Args& ... args) {
                lvl = update_log_level(lvl);
                if (!should_log(lvl)) {
                    return;
                }

                try {
                    fmt::memory_buffer buff;
                    fmt::format_to(buff, args...);
                    Log log = Log(lvl, fmt_help::to_string_view(buff));

                    std::lock_guard lock(mutex);
                    fmt::memory_buffer fmt_buff;
                    formatter->fmt(log, fmt_buff);

                    sink_it(fmt_help::to_string_view(fmt_buff));
                }
                SCAR_LOGGER_CATCH_EXCEPTION;

                throw_if_critical(lvl);
            }

            template<typename... Args>
            inline void trace(const char* msg, const Args& ... args) { log(Trace, msg, args...); }
            template<typename T>
            inline void trace(const T& msg) { log(Trace, msg); }

            template<typename... Args>
            inline void debug(const char* msg, const Args& ... args) { log(Debug, msg, args...); }
            template<typename T>
            inline void debug(const T& msg) { log(Debug, msg); }

            template<typename... Args>
            inline void info(const char* msg, const Args& ... args) { log(Info, msg, args...); }
            template<typename T>
            inline void info(const T& msg) { log(Info, msg); }

            template<typename... Args>
            inline void warn(const char* msg, const Args& ... args) { log(Warning, msg, args...); }
            template<typename T>
            inline void warn(const T& msg) { log(Warning, msg); }

            template<typename... Args>
            inline void error(const char* msg, const Args& ... args) { log(Error, msg, args...); }
            template<typename T>
            inline void error(const T& msg) { log(Error, msg); }

            template<typename... Args>
            inline void critical(const char* msg, const Args& ... args) { log(Critical, msg, args...); }
            template<typename T>
            inline void critical(const T& msg) { log(Critical, msg); }

            void set_formatter(const fmt_ptr_t& fmt);
            inline std::vector<sink_ptr_t>& get_sinks() { return sinks; }

            /* Updates the logger's options according to the given parsed arguments. */
            void apply_options(const args::ParsedArgs& args);
        };

    }
}