#pragma once
#include <sstream>

namespace version {
    constexpr auto MAJOR = 0;
    constexpr auto MINOR = 0;
    constexpr auto PATCH = 1;
}

inline std::string version_str() {
    return (std::stringstream() << "v" << version::MAJOR << "." << version::MINOR << "." << version::PATCH).str();
}