#pragma once
#include "format/format_help.hpp"
#include <string>
#include <unordered_map>

namespace scar {
    namespace log {

        enum LogLevel {
            Off,
            Trace,
            Debug,
            Info,
            Warning,
            Error,
            Critical
        };

        static const std::unordered_map<LogLevel, std::string> loglevels{
            { Trace, "trace" },
            { Debug, "debug" },
            { Info, "info" },
            { Warning, "warning" },
            { Error, "error" },
            { Critical, "error" },
            { Off, "" }
        };

        using errcode_t = uint16_t;

        struct Log {

            LogLevel level;
            std::string_view payload;
            errcode_t errcode;

            Log(LogLevel lvl, std::string_view msg, errcode_t code = 0) :
                level(lvl),
                payload(msg),
                errcode(code)
            {}
            Log(const Log& other) = default;
        };

    }
}