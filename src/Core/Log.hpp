#pragma once
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

#define SCAR_TRACE(...) ::scar::Log::GetLogger()->trace(__VA_ARGS__)
#define SCAR_INFO(...)  ::scar::Log::GetLogger()->info(__VA_ARGS__)
#define SCAR_WARN(...)  ::scar::Log::GetLogger()->warn(__VA_ARGS__)
#define SCAR_ERROR(...) ::scar::Log::GetLogger()->error(__VA_ARGS__)
#define SCAR_CRITICAL(...) ::scar::Log::GetLogger()->critical(__VA_ARGS__);
