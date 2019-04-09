#pragma once
#include "global.hpp"
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

/* Construct a new unique pointer of type T, using the passed arguments. */
template<typename T, typename... Args>
constexpr unique<T> new_unique(Args... args) {
    return std::make_unique<T>(args...);
}

/* Construct a new shared pointer of type T, using the passed arguments. */
template<typename T, typename... Args>
constexpr shared<T> new_shared(Args... args) {
    return std::make_shared<T>(args...);
}

/* Construct a new unique pointer to the passed value.
Accessing the original data after uniquifying would be undefined behavior. */
template<typename T>
constexpr unique<T> uniquify(T& val) {
    return new_unique<T>(mv(val));
}

/* Construct a new shared pointer to the passed value.
Accessing the original data after uniquifying would be undefined behavior. */
template<typename T>
constexpr shared<T> share(T& val) {
    return new_shared<T>(mv(val));
}


/* Return a string with some character repeated 'n' times. */
inline std::string repeat_char(int n, char c) {
    return std::string(n, c);
}