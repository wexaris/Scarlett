#pragma once

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

#include <cstdio>
#include <mutex>

namespace scar {
    namespace log {

        struct console_stdout {
            static inline FILE* stream() {
                return stdout;
            }
#ifdef _WIN32
            static inline HANDLE handle()
            {
                return GetStdHandle(STD_OUTPUT_HANDLE);
            }
#endif
        };

        struct console_stderr {
            static inline FILE* stream() {
                return stderr;
            }
#ifdef _WIN32
            static inline HANDLE handle()
            {
                return GetStdHandle(STD_ERROR_HANDLE);
            }
#endif
        };

        struct console_mutex {
            using mutex_t = std::mutex;
            static inline mutex_t& mutex() {
                static mutex_t mutex;
                return mutex;
            }
        };

        struct console_nullmutex {
            class mutex_t {
                inline void lock() {}
                inline void unlock() {}
                inline bool try_unlock() { return true; }
            };
            static inline mutex_t& mutex() {
                static mutex_t mutex;
                return mutex;
            }
        };

    }
}