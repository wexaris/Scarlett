#pragma once
#include "registry.hpp"

namespace scar {
    namespace log {

        static inline Registry& get_registry() {
            return Registry::instance();
        }

        static inline logger_ptr_t get_default() {
            return get_registry().get_default();
        }

        static inline logger_ptr_t get(std::string_view name) {
            return get_registry().get(name);
        }

    }
}