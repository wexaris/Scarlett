#pragma once
#include "Parse/Lex/SourceFile.hpp"
#include "Parse/Lex/Codepoint.hpp"
#include "Parse/Span.hpp"

namespace scar {

    class UTFReader {
    public:
        explicit UTFReader(const std::string& path);

        void Bump(unsigned int n = 1);

        Codepoint GetCurr() const { return m_CurrentCodepoint; }
        Codepoint GetNext() const { return m_NextCodepoint; }

        SourceFile* GetSourceFile()             { return m_SourceFile; }
        const SourceFile* GetSourceFile() const { return m_SourceFile; }
        TextPosition GetPosition() const { return m_CurrentPosition; }
        bool IsEOF() const               { return m_IsEOF; }

    private:
        SourceFile* m_SourceFile;
        TextPosition m_LastPosition;
        TextPosition m_CurrentPosition;
        Codepoint m_CurrentCodepoint;
        Codepoint m_NextCodepoint;
        size_t m_NextIndex = 0;
        bool m_IsEOF = false;

        char GetNextByte();
        Codepoint GetNextCodepoint();
    };

}