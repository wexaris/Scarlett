#pragma once
#include "files/source_file.hpp"
#include "fmt/format.h"

namespace scar {

    struct TextPos {
        size_t line = 1;
        size_t col = 1;
        size_t idx = 0;

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

    struct Span {
        source::file_ptr_t file;
        TextPos lo, hi;

        Span() = default;
        Span(source::file_ptr_t sf, const TextPos& lo, const TextPos& hi) :
            file(sf),
            lo(lo),
            hi(hi)
        {}
    };

}

template<>
struct fmt::formatter<scar::Span> {

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::Span& sp, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}:{}:{}-{}:{}",
            sp.file->name(),
            sp.lo.line, sp.lo.col,
            sp.hi.line, sp.hi.col);
    }
};