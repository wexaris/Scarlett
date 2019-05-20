#include "registry.hpp"

namespace scar {
    namespace log {

        Registry::Registry() {
            auto sink = std::make_shared<default_sink_t>();
            default_logger = std::make_shared<Logger>("", std::move(sink));
            loggers[""] = default_logger;
        }

        Registry::~Registry() {
            clear();
        }

        void Registry::register_logger(logger_ptr_t logger) {
            if (loggers.find(logger->name) != loggers.end()) {
                default_logger->warn("logger '{}' already registered", logger->name);
                return;
            }
            loggers[logger->name] = std::move(logger);
        }

        void Registry::add(logger_ptr_t logger) {
            std::lock_guard<std::mutex> lock(mutex);
            register_logger(std::move(logger));
        }

        void Registry::remove(std::string_view name) {
            std::lock_guard<std::mutex> lock(mutex);
            loggers.erase(name);
            if (default_logger != nullptr && default_logger->name == name) {
                default_logger.reset();
            }
        }

        void Registry::clear() {
            std::lock_guard<std::mutex> lock(mutex);
            loggers.clear();
            default_logger.reset();
        }

        logger_ptr_t Registry::get(std::string_view name) {
            std::lock_guard<std::mutex> lock(mutex);
            auto logger_iter = loggers.find(name);
            return logger_iter == loggers.end() ? nullptr : logger_iter->second;
        }

        logger_ptr_t& Registry::get_default() {
            std::lock_guard<std::mutex> lock(mutex);
            return default_logger;
        }

        void Registry::set_default(logger_ptr_t logger) {
            std::lock_guard<std::mutex> lock(mutex);
            if (default_logger != nullptr) {
                loggers.erase(default_logger->name);
            }
            if (logger != nullptr) {
                loggers[logger->name] = logger;
            }
            default_logger = std::move(logger);
        }

        void Registry::flush_all() {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& logger : loggers) {
                logger.second->flush();
            }
        }

    }
}