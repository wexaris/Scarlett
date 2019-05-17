#pragma once
#include "sinks/sink.hpp"
#include <functional>
#include <atomic>
#include <memory>

namespace scar {
    namespace args { struct ParsedArgs; }

    namespace log {

#define SCAR_LOGGER_CATCH_EXCEPTION catch (const std::exception& e) { internal_err(e.what()); }

        using sink_ptr_t = std::shared_ptr<sinks::SinkBase>;
        using sink_list_t = std::initializer_list<sink_ptr_t>;
        using err_handler_t = std::function<void(std::string_view msg)>;

        class Logger {

        private:
            std::vector<sink_ptr_t> sinks;
            std::atomic<size_t> error_count = 0;

            virtual void sink_it(const Log& msg);
            virtual void flush_();

            void throw_if_critical(LogLevel lvl);

            void internal_err(std::string_view msg) const;

            LogLevel update_log_level(LogLevel lvl);
            bool should_log(LogLevel lvl) const;

            inline void increase_err_count() { error_count++; }


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

            template<typename... Args>
            inline void log(LogLevel lvl, std::string_view msg, const Args& ... args) {
                lvl = update_log_level(lvl);
                if (!should_log(lvl)) {
                    return;
                }

                try {
                    fmt::memory_buffer buff;
                    fmt::format_to(buff, msg, args...);
                    Log log = Log(lvl, fmt_help::to_string_view(buff));
                    sink_it(log);
                }
                SCAR_LOGGER_CATCH_EXCEPTION;

                throw_if_critical(lvl);
            }

            // Log message with type T which can be converted to string_view
            template<class T, typename std::enable_if<
                std::is_convertible<T, std::string_view>::value, T>::type* = nullptr>
                void log(LogLevel lvl, const T & msg) {
                lvl = update_log_level(lvl);
                if (!should_log(lvl)) {
                    return;
                }
                try {
                    Log log = Log(lvl, msg);
                    sink_it(log);
                }
                SCAR_LOGGER_CATCH_EXCEPTION;

                throw_if_critical(lvl);
            }

            // Log message with type T which can't be converted to string_view
            template<class T, typename std::enable_if<
                !std::is_convertible<T, std::string_view>::value, T>::type* = nullptr>
                void log(LogLevel lvl, const T & msg) {
                lvl = update_log_level(lvl);
                if (!should_log(lvl)) {
                    return;
                }
                try {
                    fmt::memory_buffer buff;
                    fmt::format_to(buff, "{}", msg);
                    Log log = Log(lvl, fmt_help::to_string_view(buff));
                    sink_it(log);
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

            inline void set_formatter(formatter_ptr_t fmt);

            inline std::vector<sink_ptr_t>& get_sinks()             { return sinks; }
            inline size_t get_err_count() const                     { return error_count; }

            /* Updates the logger's options according to the given parsed arguments. */
            void apply_options(const args::ParsedArgs& args);
        };

    }
}