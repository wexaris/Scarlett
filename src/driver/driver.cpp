#include "driver.hpp"
#include "front/parser.hpp"
#include "log/logging.hpp"

namespace scar {

    void Session::apply_args() {
        log::get_default()->apply_options(args);
    }

    void Driver::run() {
        Parser parser = Parser(Session::instance().args.in[0]);

        parser.parse();
    }

}