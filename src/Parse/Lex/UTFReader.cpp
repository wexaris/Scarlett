#include "scarpch.hpp"
#include "Parse/Lex/UTFReader.hpp"

namespace scar {

    UTFReader::UTFReader(const std::string& path) :
        m_SourceFile(SourceMap::Load(path))
    {
        auto temp = GetNextCodepoint();
        std::swap(m_NextCodepoint, temp);
        Bump();
    }

    void UTFReader::Bump(unsigned int n) {
        for (; n > 0; n--) {
            m_LastPosition = m_CurrentPosition;

            if (m_CurrentCodepoint == '\n') {
                m_CurrentPosition.Line++;
                m_CurrentPosition.Col = 1;
            }
            else {
                m_CurrentPosition.Col++;
            }

            m_CurrentCodepoint = m_NextCodepoint;
            m_CurrentPosition.Index = m_NextIndex++;

            if (m_CurrentCodepoint.IsEOF()) {
                m_IsEOF = true;
                return;
            }

            m_NextCodepoint = GetNextCodepoint();
        }
    }

    char UTFReader::GetNextByte() {
        if (IsEOF()) {
            return '\0';
        }
        return m_SourceFile->GetChar(m_NextIndex);
    }

    Codepoint UTFReader::GetNextCodepoint() {
        uint8_t v1 = GetNextByte();

        if (v1 < 128) {
            return Codepoint(v1);
        }
        else if ((v1 & 0xC0) == 0x80) { // Invalid
            return Codepoint(0xFFFE);
        }
        else if ((v1 & 0xE0) == 0xC0) { // Two bytes
            uint8_t e1 = GetNextByte();
            if ((e1 & 0xC0) != 0x80) { return { 0xFFFE }; }

            uint32_t outval =
                ((v1 & 0x1F) << 6) |
                ((e1 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else if ((v1 & 0xF0) == 0xE0) { // Three bytes
            uint8_t e1 = GetNextByte();
            if ((e1 & 0xC0) != 0x80) { return { 0xFFFE }; }
            uint8_t e2 = GetNextByte();
            if ((e2 & 0xC0) != 0x80) { return { 0xFFFE }; }

            uint32_t outval =
                ((v1 & 0x0F) << 12) |
                ((e1 & 0x3F) << 6) |
                ((e2 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else if ((v1 & 0xF8) == 0xF0) { // Four bytes
            uint8_t e1 = GetNextByte();
            if ((e1 & 0xC0) != 0x80) { return { 0xFFFE }; }
            uint8_t e2 = GetNextByte();
            if ((e2 & 0xC0) != 0x80) { return { 0xFFFE }; }
            uint8_t e3 = GetNextByte();
            if ((e3 & 0xC0) != 0x80) { return { 0xFFFE }; }

            uint32_t outval =
                ((v1 & 0x07) << 18) |
                ((e1 & 0x3F) << 12) |
                ((e2 & 0x3F) << 6) |
                ((e3 & 0x3F) << 0);
            return Codepoint(outval);
        }
        else {
            throw ParseError(ErrorCode::InvalidUTF8, [&]() { SCAR_ERROR("{}: invalid UTF-8; code is too long", Span(m_SourceFile, m_LastPosition, m_CurrentPosition)); });
        }
    }

}