#include "ast.hpp"
#include "util/interner.hpp"

namespace ast {

    InternedString Name::get_string() const {
        return global::interner->get(*this);
    }

}