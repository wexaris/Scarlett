#include "scarpch.hpp"
#include "Core/Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace scar {

    Ref<spdlog::logger> Log::s_Logger;

    void Log::Init() {
        // Create sinks
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_pattern("%^%v%$");

        // Create loggers
        s_Logger = MakeRef<spdlog::logger>("scar", sink);
        s_Logger->set_level(spdlog::level::trace);
        s_Logger->flush_on(spdlog::level::trace);
        spdlog::register_logger(s_Logger);
    }
}