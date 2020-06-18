#pragma once
#include <exception>
#include <functional>

namespace scar {

    enum class ErrorCode : int16_t {
        Unknown = -1,

        // Pre-compile errors
        CommandLineError = 1,
        SourceFileError,

        // Codepoint
        InvalidUTF8,
        // Lexer
        InvalidLiteral,
        // Parser
        EOFInComment,
        UnexpectedToken,
    };

    class CompilerException : public std::exception {
    public:
        const ErrorCode Code;
        CompilerException(ErrorCode code, std::function<void(void)> onCatch) : Code(code), m_OnCatch(onCatch) {}
        void OnCatch() const { m_OnCatch(); }

    private:
        const std::function<void(void)> m_OnCatch;
    };

    // Error missing some information before it can be logged
    // Should be intercepted and handled appropriately
    class RogueError : public CompilerException {
    public:
        const std::string Message;
        RogueError(ErrorCode code, const std::string& message) : CompilerException(code, {}), Message(message) {}
    };

    // Error used for recovery while parsing
    class ParseError : public CompilerException {
    public:
        ParseError(ErrorCode code, std::function<void(void)> onCatch) : CompilerException(code, onCatch) {}
    };

    // Error used for stopping compilation
    class Terminate : public CompilerException {
    public:
        Terminate(ErrorCode code, std::function<void(void)> onCatch) : CompilerException(code, onCatch) {}
    };

}