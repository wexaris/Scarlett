#pragma once

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// BUILD

#ifdef _WIN32
    #define SCAR_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define SCAR_PLATFORM_LINUX
#else
    #error Unknown platform!
#endif

#ifdef SCAR_PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

#ifdef _DEBUG
    #define SCAR_DEBUG
#else
    #define SCAR_RELEASE
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// VARIABLES

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
    #define SCAR_FUNCSIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
    #define SCAR_FUNCSIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
    #define SCAR_FUNCSIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    #define SCAR_FUNCSIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    #define SCAR_FUNCSIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    #define SCAR_FUNCSIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
    #define SCAR_FUNCSIG __func__
#else
    #define SCAR_FUNCSIG "unknown function"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// SETTINGS

#ifdef SCAR_DEBUG
    #define PYRE_ENABLE_ASSERTS
    #define SCAR_ENABLE_BUG_LOG
    #define SCAR_ENABLE_UNIMPL_LOG

    #if defined SCAR_PLATFORM_WINDOWS
        #define SCAR_DEBUGBREAK() __debugbreak()
    #elif defined SCAR_PLATFORM_LINUX
        #include <signal.h>
        #define SCAR_DEBUGBREAK() raise(SIGTRAP)
    #endif
#endif

#ifdef SCAR_ENABLE_BUG_LOG
    #define SCAR_BUG(...) { SCAR_ERROR("{}: BUG: {}", SCAR_FUNCSIG, FMT(__VA_ARGS__)); SCAR_DEBUGBREAK(); }
#else
    #define SCAR_BUG(...)
#endif

#ifdef SCAR_ENABLE_UNIMPL_LOG
    #define SCAR_UNIMPL(...) { SCAR_ERROR("{} not implemented", FMT(__VA_ARGS__)); SCAR_DEBUGBREAK(); }
#else
    #define SCAR_UNIMPL(...)
#endif

#ifdef SCAR_ENABLE_ASSERTS
    #define SCAR_ASSERT(x, msg) { if (!(x)) { PYRE_ERROR("{}: ASSERT: {}", SCAR_FUNCSIG, msg); PYRE_DEBUGBREAK(); } }
#else
    #define SCAR_ASSERT(x, msg)
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// APPLICATION

#include <memory>

namespace scar {

    template<typename T> using Ref = std::shared_ptr<T>;
    template<typename T> using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    Ref<T> MakeRef(Args&&... args) {
        return std::make_shared<T>(args...);
    }

    template<typename T, typename... Args>
    Scope<T> MakeScope(Args&&... args) {
        return std::make_unique<T>(args...);
    }
}
