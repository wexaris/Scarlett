#include "ast.hpp"
#include "interner.hpp"


namespace scar {
    namespace ast {

        interned_str_t Name::get_str() const {
            return Interner::instance().find(*this);
        }

        FunCallPrint::FunCallPrint(ast::ArgList args) :
            FunCall({ Interner::instance().intern("print") }, std::move(args))
        {}

    }
}