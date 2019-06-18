#include "error.hpp"
#include "log/logging.hpp"

namespace scar {
    namespace err {

        ParseError::ParseError(log::LogLevel lvl, std::string msg) :
            lvl(lvl),
            msg(std::move(msg))
        {}

        void ParseError::emit_() const {
            log::log(lvl, msg);
        }

    }
}