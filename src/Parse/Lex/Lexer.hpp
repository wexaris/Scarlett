#pragma once
#include "Parse/Lex/UTFReader.hpp"
#include "Parse/Interner.hpp"
#include "Parse/Token.hpp"

namespace scar {

    class Lexer {
    public:
        explicit Lexer(const std::string& path);

        // Return a TokenStream of the current file
        TokenStream Lex();

        // Get the current SourceFile
        SourceFile* GetSourceFile()             { return m_Reader.GetSourceFile(); }
        const SourceFile* GetSourceFile() const { return m_Reader.GetSourceFile(); }

        unsigned int GetErrorCount() const { m_ErrorCount; }

    private:
        UTFReader m_Reader;
        TextPosition m_TokenStartPosition;

        unsigned int m_ErrorCount = 0;

        void Bump(unsigned int n = 1);
        // Get the current Codepoint
        Codepoint GetCurr() const { return m_Reader.GetCurr(); }
        // Get the next Codepoint
        Codepoint GetNext() const { return m_Reader.GetNext(); }

        // Get the current Token's position
        TextPosition GetPosition() const { return m_Reader.GetPosition(); }
        // Get the current Token's span
        Span GetSpan() const { return Span(GetSourceFile(), m_TokenStartPosition, GetPosition()); }
        // Get the current Token's raw string
        std::string_view GetString() const { return GetSourceFile()->GetString(m_TokenStartPosition.Index, GetPosition().Index - m_TokenStartPosition.Index); }

        void Error(std::string_view msg);
        [[noreturn]] void ThrowError(std::string_view msg);

        // Main tokenization function
        Token GetNextToken();

        // Read past whitespace and comments
        void SkipWhitespaceAndComments();

        // Main number tokenization function
        Token LexNumber();
        // Read past digits of a certain base
        void ReadDigits(unsigned int base);
        // Read past floating point fraction
        void ReadFraction();
        // Read past floating point exponent
        void ReadExponent();

        // Read past an escaped hex code, return it as a Codepoint
        Codepoint ReadHexEscape(unsigned int length, Codepoint delim);
    };

}