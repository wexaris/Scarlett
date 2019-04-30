#pragma once
#include "sink.hpp"
#include "log/details.hpp"
#include "fmt/format.h"
#include "fmt/core.h"

namespace scar {
    namespace log {
        namespace sinks {

            template<typename Mutex>
            class MSVCSink : public ThreadSafeSink<Mutex> {

            protected:
                void sink_it_(const details::log_msg& msg) override {
                    fmt::memory_buffer formatted;
                    sink::formatter_->format(msg, formatted);
                    OutputDebugStringA(fmt::to_string(formatted).c_str());
                }

                inline void flush_() override {}

            public:
                explicit MSVCSink() = default;
            };

        }
    }
}