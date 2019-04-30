#pragma once
#include "logger.hpp"
#include "sinks/stdout_sink.hpp"
#include "util/logger_ex.hpp"
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

            Registry() {
                auto sink = std::make_shared<default_sink_t>();
                default_logger = std::make_shared<Logger>("", std::move(sink));
                loggers[""] = default_logger;
            }
            ~Registry() = default;

            inline void register_logger(logger_ptr_t logger) {
                if (loggers.find(logger->name) != loggers.end()) {
                    throw LoggerException("logger '" + std::string(logger->name) + "' already registered");
                }
                loggers[logger->name] = std::move(logger);
            }

        public:
            static Registry& instance() {
                static Registry registry;
                return registry;
            }

            Registry(const Registry & other) = delete;
            Registry& operator=(const Registry & other) = delete;

            void add(logger_ptr_t logger) {
                std::lock_guard<std::mutex> lock(mutex);
                register_logger(std::move(logger));
            }

            void remove(std::string_view name) {
                std::lock_guard<std::mutex> lock(mutex);
                loggers.erase(name);
                if (default_logger != nullptr && default_logger->name == name) {
                    default_logger.reset();
                }
            }

            void clear() {
                std::lock_guard<std::mutex> lock(mutex);
                loggers.clear();
                default_logger.reset();
            }

            logger_ptr_t get(std::string_view name) {
                std::lock_guard<std::mutex> lock(mutex);
                auto logger_iter = loggers.find(name);
                return logger_iter == loggers.end() ? nullptr : logger_iter->second;
            }

            logger_ptr_t& get_default() {
                std::lock_guard<std::mutex> lock(mutex);
                return default_logger;
            }

            void set_default(logger_ptr_t logger) {
                std::lock_guard<std::mutex> lock(mutex);
                if (default_logger != nullptr) {
                    loggers.erase(default_logger->name);
                }
                if (logger != nullptr) {
                    loggers[logger->name] = logger;
                }
                default_logger = std::move(logger);
            }

            void flush_all() {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& logger : loggers) {
                    logger.second->flush();
                }
            }
        };

    }
}