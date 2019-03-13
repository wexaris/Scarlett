#pragma once
#include "common.hpp"

struct SourceFile;

struct TextPos {
    uint line = 1;
    uint col = 1;
    uint idx = 0;

    inline TextPos operator+(const TextPos& other) {
        TextPos pos;
        pos.line = line + other.line;
        pos.col = col + other.col;
        pos.idx = idx + other.idx;
        return pos;
    }
    inline TextPos operator-(const TextPos& other) {
        TextPos pos;
        pos.line = line - other.line;
        pos.col = col - other.col;
        pos.idx = idx - other.idx;
        return pos;
    }
};

std::ostream& operator<<(std::ostream& os, const TextPos& sp);

struct Span {

    const SourceFile* source = nullptr;
    TextPos lo, hi;

    constexpr Span() = default;
    constexpr Span(const SourceFile* sf, const TextPos& lo, const TextPos& hi) :
        source(sf),
        lo(lo),
        hi(hi)
    {}
};

std::ostream& operator<<(std::ostream& os, const Span& sp);