#include "driver.hpp"
#include "parse/parser.hpp"
#include "ast/visitor/llvm_visitor.hpp"
#include "ast/visitor/llvm_opt_visitor.hpp"
#include "util/error.hpp"

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
		// The logger is often used to track errors across the
		// whole compilation process, so it is called quite often.
		// To make this less verbose, we store a reference.
#define CHECK_LOGGER_THROW(e) \
	Session::get().logger().emit_delayed(); \
		if (Session::get().logger().had_error()) { \
			Session::get().logger().finish(); \
			throw err::FatalError(e); \
		}

		// Parse a token stream of the input file
		Lexer lexer = Lexer(Session::get().args().in[0]);
		auto tokens = lexer.lex();
        Parser parser = Parser(tokens);
        auto ast = parser.parse();
		CHECK_LOGGER_THROW(err_codes::ERR_FAILED_PARSE);

		// Generate LLVM IR
        auto codegen = ast::LLVMVisitor(Session::get().logger());
		ast.accept(codegen);
		CHECK_LOGGER_THROW(err_codes::ERR_FAILED_CODEGEN);

		// Optimize the LLVM IR
		auto opt = ast::LLVMOptimizeVisitor(codegen.module);
		ast.accept(opt);
		CHECK_LOGGER_THROW(err_codes::ERR_FAILED_OPTIMIZE);

		// Emit the LLVM IR
		codegen.module->print(llvm::errs(), nullptr);
    }

}