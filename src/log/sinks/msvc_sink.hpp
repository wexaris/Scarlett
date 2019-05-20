#pragma once
#include "sink.hpp"
#include "console_vars.hpp"
#include "fmt/format.h"
#include "fmt/core.h"

namespace scar {
    namespace log {
        namespace sinks {

            template<typename Mutex>
            class MSVCSink : public SinkBase {

            public:
                explicit MSVCSink() = default;

                inline void log(std::string_view msg) final override {
                    OutputDebugStringA(msg);
                }

                inline void flush() final override {}
            };

        }
    }
}