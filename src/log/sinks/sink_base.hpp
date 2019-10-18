#pragma once
#include <string>

namespace scar {
    namespace log {

        namespace sinks {

            class SinkBase {

            public:
                SinkBase() = default;
                virtual ~SinkBase() = default;

                virtual void log(std::string_view msg) = 0;
                virtual void flush() = 0;
            };

        }
    }
}