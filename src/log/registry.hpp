#pragma once
#include "logger.hpp"
#include "sinks/stdout_sink.hpp"
#include <unordered_map>

namespace scar {
    namespace log {

        using logger_ptr_t = std::shared_ptr<Logger>;
        using default_sink_t = sinks::StdOutSink<console_stdout, console_mutex>;

        class Registry {

        private:
            std::mutex mutex;
            std::mutex flush_mutex;
            std::unordered_map<std::string_view, logger_ptr_t> loggers;
            logger_ptr_t default_logger;

            void register_logger(logger_ptr_t logger);

            Registry();
            ~Registry();
            Registry(const Registry& other) = delete;

            Registry& operator=(const Registry& other) = delete;

        public:
            static Registry& instance() {
                static Registry registry;
                return registry;
            }

            void add(logger_ptr_t logger);
            void remove(std::string_view name);
            void clear();

            logger_ptr_t get(std::string_view name);
            logger_ptr_t& get_default();
            void set_default(logger_ptr_t logger);


            void flush_all();
        };

    }
}