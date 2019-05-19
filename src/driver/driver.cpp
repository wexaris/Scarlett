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

        if (parser.had_error()) {
            auto err_num = parser.err_count();
            std::string err_count_str = fmt::format("{} error", err_num);
            if (err_num > 1) {
                err_count_str += "s";
            }
            log::get_default()->error("build failed due to {}", err_count_str);
        }
    }

}