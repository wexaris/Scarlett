#pragma once
#include "log/log.hpp"
#include "fmt/format.h"
#include "fmt/core.h"
#include <string>

namespace scar {
    namespace log {
        namespace format {

            class FormatterBase {

            public:
                FormatterBase() = default;
                virtual ~FormatterBase() = default;

                virtual void fmt(const Log& msg, fmt::memory_buffer& buff) const = 0;
            };

        }
    }
}