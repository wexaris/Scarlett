#pragma once
#include <exception>
#include <functional>

namespace scar {

    class CompilerError : public std::exception {
    public:
        CompilerError(const std::string& message) : m_Message(message) {}
        virtual ~CompilerError() = default;

        void OnCatch() const { Session::Error(m_Message); }

        const char* what() const noexcept override { return m_Message.c_str(); }

    private:
        std::string m_Message;
    };

}