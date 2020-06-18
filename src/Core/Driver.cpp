#include "scarpch.hpp"
#include "Core/Driver.hpp"
#include "Parse/Parser.hpp"
#include "Parse/AST/LLVMVisitor.hpp"
#include "Parse/AST/TypeCheckVisitor.hpp"
#include "Parse/AST/PrintVisitor.hpp"

namespace scar {

    Session* Session::s_Instance;

    void Session::Create(int argc, const char* argv[]) {
        s_Instance = new Session(argc, argv);
    }

    Session::Session(int argc, const char* argv[]) {
        if (argc == 1) {
            throw Terminate(ErrorCode::CommandLineError, [&]() { SCAR_ERROR("no input file specified!"); } );
        }
        else {
            InputFile = argv[1];
        }
    }

    bool Driver::m_Initialized        = false;
    int Driver::m_ReturnState         = 0;
    unsigned int Driver::m_ErrorCount = 0;

    void Driver::Init(int argc, const char* argv[]) {
        try {
            Log::Init();
            Session::Create(argc, argv);

            m_Initialized = true;
        }
        catch (Terminate& e) {
            e.OnCatch();
            m_ReturnState = (int)e.Code;
        }
    }

    void Driver::Compile() {
        if (m_ReturnState != 0) {
            return;
        }

        if (!m_Initialized) {
            SCAR_BUG("Driver not initialized!");
            return;
        }

        try {
            Parser parser(Session::Get().InputFile);
            auto ast = parser.Parse();

            ast::TypeCheckVisitor typeCheck;
            ast->Accept(typeCheck);

            ast::PrintVisitor print;
            ast->Accept(print);

            if (!parser.GetErrorCount()) {
                ast::LLVMVisitor codegen;
                ast->Accept(codegen);
                codegen.Print();
            }
        }
        catch (CompilerException& e) {
            e.OnCatch();
            m_ReturnState = (int)e.Code;
            return;
        }
    }

}