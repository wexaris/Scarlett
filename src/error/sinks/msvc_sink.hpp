#pragma once
#include "sink_base.hpp"
#include "console_vars.hpp"

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

            using MSVC = sinks::MSVCSink<console_nullmutex>;

        }
    }
}