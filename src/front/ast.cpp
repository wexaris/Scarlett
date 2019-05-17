#include "ast.hpp"
#include "interner.hpp"


namespace scar {
    namespace ast {

        interned_str_t Ident::get_str() const {
            return Interner::instance().find(*this);
        }

        ExprFunCallPrint::ExprFunCallPrint(unique<Expr> args) :
            ExprFunCall(Interner::instance().intern("print"), std::move(args))
        {}

    }
}