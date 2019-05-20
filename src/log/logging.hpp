#pragma once
#include "registry.hpp"

namespace scar {
    namespace log {

        static inline Registry& get_registry() {
            return Registry::instance();
        }

        static inline logger_ptr_t get(std::string_view name) {
            return get_registry().get(name);
        }
        static inline logger_ptr_t get_default() {
            return get_registry().get_default();
        }

        template<typename... Args>
        static inline void log(LogLevel lvl, const Args&... args) {
            get_default()->log(lvl, args...);
        }

        template<typename... Args>
        static inline void trace(const char* msg, const Args&... args) {
            get_default()->trace(msg, args...);
        }
        template<typename T>
        static inline void trace(const T& msg) {
            get_default()->trace(msg);
        }

        template<typename... Args>
        static inline void debug(const char* msg, const Args&... args) {
            get_default()->debug(msg, args...);
        }
        template<typename T>
        static inline void debug(const T& msg) {
            get_default()->debug(msg);
        }

        template<typename... Args>
        static inline void info(const char* msg, const Args&... args) {
            get_default()->info(msg, args...);
        }
        template<typename T>
        static inline void info(const T& msg) {
            get_default()->info(msg);
        }

        template<typename... Args>
        static inline void warn(const char* msg, const Args&... args) {
            get_default()->warn(msg, args...);
        }
        template<typename T>
        static inline void warn(const T& msg) {
            get_default()->warn(msg);
        }

        template<typename... Args>
        static inline void error(const char* msg, const Args&... args) {
            get_default()->error(msg, args...);
        }
        template<typename T>
        static inline void error(const T& msg) {
            get_default()->error(msg);
        }

        template<typename... Args>
        static inline void critical(const char* msg, const Args&... args) {
            get_default()->critical(msg, args...);
        }
        template<typename T>
        static inline void critical(const T& msg) {
            get_default()->critical(msg);
        }

        static inline void set_formatter(const fmt_ptr_t& fmt) {
            return get_default()->set_formatter(fmt);
        }
        static inline std::vector<sink_ptr_t>& get_sinks() {
            return get_default()->get_sinks();
        }

        static inline void apply_options(const args::ParsedArgs& args) {
            get_default()->apply_options(args);
        }

        static inline LogLevel update_log_level(LogLevel lvl) {
            return get_default()->update_log_level(lvl);
        }
        static inline bool should_log(LogLevel lvl) {
            return get_default()->should_log(lvl);
        }

    }
}