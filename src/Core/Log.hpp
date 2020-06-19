#pragma once
#include "Core/Session.hpp"
#include "Core/Error.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>

namespace scar {

    class Log {
    public:
        static void Init();

        static Ref<spdlog::logger>& GetLogger() { return s_Logger; }

    private:
        static Ref<spdlog::logger> s_Logger;
    };

}

#define FMT(...) ::fmt::format(__VA_ARGS__)

#define SCAR_TRACE(...) ::scar::Session::Trace(FMT(__VA_ARGS__))
#define SCAR_INFO(...)  ::scar::Session::Info(FMT(__VA_ARGS__))
#define SCAR_WARN(...)  ::scar::Session::Warn(FMT(__VA_ARGS__))
#define SCAR_ERROR(...) throw ::scar::CompilerError(FMT(__VA_ARGS__));
#define SCAR_CRITICAL(...) { SCAR_ERROR(__VA_ARGS__); throw std::runtime_error(FMT(__VA_ARGS__)); }
