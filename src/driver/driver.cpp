#include "driver.hpp"
#include "parse/parser.hpp"
#include "ast/visitor/llvm_visitor.hpp"
#include "log/logging.hpp"

namespace scar {

    void Session::apply_args() {
        log::apply_options(args);
    }

    void Session::init(int argc, const char** argv) {
        instance().args = scar::args::parse(argc, argv);
        instance().apply_args();
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void Driver::run() {
        Parser parser = Parser(Session::instance().get_args().in[0]);

        auto ast = parser.parse();

        if (parser.had_error()) {
            auto err_num = parser.err_count();
            std::string err_count_str = fmt::format("{} error", err_num);
            if (err_num > 1) {
                err_count_str += "s";
            }
            log::error("build failed due to {}", err_count_str);
        }

        auto visitor = ast::LLVMVisitor(Session::instance().get_symbols());

        ast.accept(visitor);

        visitor.module.print(llvm::errs(), nullptr);
    }

}