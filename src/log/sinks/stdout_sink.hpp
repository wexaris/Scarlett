#pragma once
#include "sink.hpp"
#include "console_vars.hpp"
#include "util/early_exit.hpp"
#include <iostream>

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

                void log(const Log& msg) final override {
                    std::lock_guard<mutex_t> lock(mutex);
                    fmt::memory_buffer fmt_buff;
                    formatter->fmt(msg, fmt_buff);
                    fwrite(fmt_buff.data(), sizeof(char), fmt_buff.size(), file);
                    fflush(file);
                }

                inline void flush() final override {
                    std::lock_guard<mutex_t> lock(mutex);
                    fflush(file);
                }

                void set_formatter(formatter_ptr_t fmt) final override {
                    std::lock_guard<mutex_t> lock(mutex);
                    formatter = std::move(fmt);
                }
            };

        }
    }
}