#pragma once
#include <memory>
#include <string>

namespace scar {

    using interned_str_t = std::shared_ptr<std::string>;

    namespace ast {


        struct NodeBase {};

        struct Ident {
            size_t name;
        };

        struct Name {
            size_t id;

            Name(size_t id = 0) : id(id) {}

            interned_str_t get_string() const;
            inline Ident to_ident() const { return Ident{ id }; }
        };

        using root_t = std::shared_ptr<NodeBase>;

    }
}