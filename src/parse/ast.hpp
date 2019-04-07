#pragma once
#include "common.hpp"

namespace ast {

    struct Ident {
        size_t name;
    };

    struct Name {
        size_t id;

        Ident to_ident() const {
            return Ident{ id };
        }
    };

}