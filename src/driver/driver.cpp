#include "driver.hpp"
#include "front/parser.hpp"
#include "log/logging.hpp"

namespace scar {

    void Session::apply_args() {
        log::get_default()->apply_options(args);
    }

    void Driver::run() {
        Lexer lexer = Lexer(Session::instance().args.in[0]);
        
        auto tok = lexer.next_token();
        if (!tok.is_eof()) {
            do {
                log::get_default()->info(tok);
                tok = lexer.next_token();
            } while (!tok.is_eof());
        }
    }

}