#include "scarpch.hpp"
#include "Core/Driver.hpp"
#include "Core/Session.hpp"
#include "Parse/Parser.hpp"
#include "Parse/AST/LLVMVisitor.hpp"
#include "Parse/AST/TypeCheckVisitor.hpp"
#include "Parse/AST/PrintVisitor.hpp"

namespace scar {

    void Driver::Init(const std::vector<const char*>& args) {
        scar::Log::Init();
        scar::Session::Init(args);
    }

    void Driver::Compile() {
        if (!Session::IsGood())
            return;

        try {
            // TODO: Pass Session into these passes.
            // Session should provide one consistent error logging system,
            // as well as handle error counting, so we don't have to query
            // every visitor and parse individually.
            Parser parser(Session::GetInputFile());
            auto ast = parser.Parse();

            ast::TypeCheckVisitor typeCheck;
            ast->Accept(typeCheck);

            //ast::PrintVisitor print;
            //ast->Accept(print);

            if (Session::IsGood()) {
                ast::LLVMVisitor codegen;
                ast->Accept(codegen);
                codegen.Print();
            }
        }
        catch (CompilerError& e) {
            e.OnCatch();
        }
    }

    void Driver::Exit() {
        if (Session::IsGood()) {
            SCAR_INFO("Compilation successful");
        }
        else {
            SCAR_INFO("Compilation failed due to {} error{}", Session::GetErrorCount(), Session::GetErrorCount() > 1 ? "s" : "");
        }
    }

}