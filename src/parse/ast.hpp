#pragma once
#include "common.hpp"

using InternedString = shared<std::string>;

namespace ast {

    struct Ident {
        size_t name;
    };

    struct Name {
        size_t id;

        inline Ident to_ident() const { return Ident{ id }; }

        InternedString get_string() const;
    };

}