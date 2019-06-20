#pragma once
#include "sinks/sinks.hpp"
#include "fmt_help.hpp"
#include <vector>
#include <memory>

namespace scar {
    namespace log {

        using sink_ptr_t = std::shared_ptr<sinks::SinkBase>;
        using sink_list_t = std::initializer_list<sink_ptr_t>;

        class Emitter {

        private:
            std::vector<sink_ptr_t> sinks;

            virtual void sink_it(std::string_view msg);
            virtual void flush_();

        public:
            Emitter(sink_ptr_t sink);
            Emitter(sink_list_t sinks);
            virtual ~Emitter() = default;

            void flush();

            inline void log(std::string_view msg) {
                sink_it(msg);
            }

            template<typename... Args>
            inline void log(std::string_view msg, const Args& ... args) {
                fmt::memory_buffer buff;
                fmt::format_to(buff, msg, args...);
                auto log = fmt_help::to_string_view(buff);

                sink_it(log);
            }

            inline std::vector<sink_ptr_t>& get_sinks() { return sinks; }
        };

    }
}