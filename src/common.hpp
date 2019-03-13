#pragma once
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

using uint = unsigned int;

template<typename T> using unique = std::unique_ptr<T>;
template<typename T> using shared = std::shared_ptr<T>;

#define mv(x) std::move(x)

#define FMT(x) (std::stringstream() << x).str()
#define LOG(x) std::cout << x << std::endl
#define ERR(x) throw std::runtime_error(x)

#define new_unique(x, ...) std::make_unique<x>(__VA_ARGS__)
#define new_shared(x, ...) std::make_shared<x>(__VA_ARGS__)

#define mk_unique(x) smart_ptr::make_unique(mv(x))
#define mk_shared(x) smart_ptr::make_shared(mv(x))


namespace smart_ptr {
    /* Move some existing object into a unique pointer.
    The original object will be invalidated. */
    template<typename T>
    constexpr unique<T> make_unique(T&& val) {
        return unique<T>(new T(mv(val)));
    }

    /* Move some existing object into a shared pointer.
    The original object will be invalidated. */
    template<typename T>
    constexpr shared<T> make_shared(T&& val) {
        return shared<T>(new T(mv(val)));
    }
}

/* Return a string with some character repeated 'n' times. */
inline std::string repeat_char(int n, char c) {
    return std::string(n, c);
}