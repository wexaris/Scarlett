#include "emitter.hpp"

namespace scar {
    namespace log {

        Emitter::Emitter(sink_ptr_t sink) :
            sinks({ std::move(sink) })
        {}

        Emitter::Emitter(sink_list_t sinks) :
            sinks(sinks.begin(), sinks.end())
        {}

        void Emitter::sink_it(std::string_view msg) {
            for (auto& sink : sinks) {
                sink->log(msg);
            }
            flush();
        }

        void Emitter::flush() {
            flush_();
        }

        void Emitter::flush_() {
            for (auto& sink : sinks) {
                sink->flush();
            }
        }

    }
}