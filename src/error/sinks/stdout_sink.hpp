#pragma once
#include "sink_base.hpp"
#include "console_vars.hpp"

namespace scar {
    namespace log {
        namespace sinks {

            template<class Stream, class Mutex>
            class StdOutSink final : public SinkBase {

            private:
                using mutex_t = typename Mutex::mutex_t;

                FILE* file;
                mutex_t& mutex;

            public:
                explicit StdOutSink() :
                    file(Stream::stream()),
                    mutex(Mutex::mutex())
                {}

                StdOutSink(const StdOutSink& other) = delete;
                StdOutSink& operator=(const StdOutSink& other) = delete;

                void log(std::string_view msg) final override {
                    std::lock_guard<mutex_t> lock(mutex);
                    fwrite(msg.data(), sizeof(char), msg.size(), file);
                    fflush(file);
                }

                inline void flush() final override {
                    fflush(file);
                }
            };

            using StdOut = sinks::StdOutSink<console_stdout, console_mutex>;
            using StdErr = sinks::StdOutSink<console_stdout, console_mutex>;

        }
    }
}