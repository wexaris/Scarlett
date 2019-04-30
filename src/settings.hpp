#pragma once

namespace scar {
    namespace version {

        constexpr int MAJOR = 0;
        constexpr int MINOR = 0;
        constexpr int PATCH = 1;

        inline std::string get_version() {
            return fmt::format("v{}.{}.{}", MAJOR, MINOR, PATCH);
        }
    }

    constexpr int BUFFER_READ_SIZE = 4096;

#ifdef NDEBUG   // RELEASE
    constexpr bool SCAR_DEBUG_ENABLED = false;
#else           // DEBUG
    constexpr bool SCAR_DEBUG_ENABLED = true;
#endif

}