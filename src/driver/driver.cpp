#include "driver.hpp"
#include "parse/parser.hpp"
#include "ast/visitor/llvm_visitor.hpp"

namespace scar {

    void Session::apply_args() {
        log_mgr.apply_options(cmd_args);
    }

    void Session::init(int argc, const char** argv) {
        cmd_args = scar::args::parse(argc, argv);
        apply_args();
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void Driver::run() {
        Parser parser = Parser(Session::get().args().in[0]);

        auto ast = parser.parse();

        if (Session::get().logger().print_error_count()) {
            return;
        }

        auto visitor = ast::LLVMVisitor(Session::get().symbols());

        ast.accept(visitor);

        visitor.module.print(llvm::errs(), nullptr);
    }

}