#pragma once
#include "log/format/default_fmt.hpp"
#include <memory>
#include <mutex>

namespace scar {
    namespace log {

        using formatter_ptr_t = std::shared_ptr<format::FormatterBase>;
        using default_formatter_t = format::DefaultFormatter;

        namespace sinks {

            class SinkBase {

            protected:
                formatter_ptr_t formatter = std::make_unique<default_formatter_t>();

            public:
                SinkBase() = default;
                explicit SinkBase(formatter_ptr_t fmt) : formatter(std::move(fmt)) {}
                virtual ~SinkBase() = default;

                virtual void log(const Log& msg) = 0;
                virtual void flush() = 0;
                virtual void set_formatter(formatter_ptr_t fmt) = 0;
            };

            template<typename Mutex>
            class ThreadSafeSink : public SinkBase {

            protected:
                Mutex mutex;

                formatter_ptr_t formatter = std::make_unique<default_formatter_t>();

                virtual void sink_it_(const Log& msg) = 0;
                virtual void flush_() = 0;
                virtual void set_formatter_(formatter_ptr_t fmt) { formatter = std::move(fmt); }

            public:
                ThreadSafeSink() : SinkBase() {}
                virtual ~ThreadSafeSink() { flush(); }

                ThreadSafeSink(const ThreadSafeSink& other) = delete;
                ThreadSafeSink& operator=(const ThreadSafeSink& other) = delete;

                virtual void log(const Log& msg) final override {
                    std::lock_guard<Mutex> lock(mutex);
                    sink_it_(msg);
                }

                virtual void flush() final override {
                    std::lock_guard<Mutex> lock(mutex);
                    flush_();
                }

                virtual void set_formatter(formatter_ptr_t fmt) final override {
                    std::lock_guard<Mutex> lock(mutex);
                    set_formatter_(std::move(fmt));
                }
            };

        }
    }
}