#pragma once
#include "Parse/Lex/SourceFile.hpp"

namespace scar {

    struct TextPosition {
        size_t Line, Col;
        size_t Index;

        TextPosition() : Line(1), Col(1), Index(0) {}
        TextPosition(size_t line, size_t col, size_t index) : Line(line), Col(col), Index(index) {}
    };

    struct Span {
        const SourceFile* File;
        size_t Line = 1, Col = 1;
        size_t Index = 0, Length = 0;

        Span() = default;
        Span(const SourceFile* file, size_t line, size_t col, size_t index, size_t length) :
            File(file),
            Line(line), Col(col),
            Index(index), Length(length) {
        }
        Span(const SourceFile* file, const TextPosition& lo, const TextPosition& hi) :
            Span(file, lo.Line, lo.Col, lo.Index, hi.Index - lo.Index) {
        }
    };

    static std::string AsString(const TextPosition& pos) {
        return FMT("{}:{}", pos.Line, pos.Col, pos.Index);
    }
    static std::string AsString(const Span& span) {
        return FMT("{}:{}:{}", span.File->GetFileName(), span.Line, span.Col);
    }

    static std::ostream& operator<<(std::ostream& os, const TextPosition& pos) {
        return os << ::scar::AsString(pos);
    }
    static std::ostream& operator<<(std::ostream& os, const Span& span) {
        return os << ::scar::AsString(span);
    }

}