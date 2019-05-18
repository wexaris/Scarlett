#include "error.hpp"
#include "log/logging.hpp"

namespace scar {
    namespace err {

        Error::Error(log::LogLevel lvl, std::string msg) :
            lvl(lvl),
            msg(std::move(msg))
        {}

        [[noreturn]] void Error::emit() const {
            log::get_default()->log(lvl, msg);
            throw *this;
        }

    }
}