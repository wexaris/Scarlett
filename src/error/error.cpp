#include "error.hpp"
#include "driver/session.hpp"

namespace scar {
    namespace err {

        ParseError::ParseError(log::Level lvl, std::string msg) :
            lvl(lvl),
            msg(std::move(msg))
        {}

        void ParseError::emit_() const {
            Session::get().logger().make_new(lvl, msg);
        }

    }
}