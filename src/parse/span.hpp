#pragma once
#include "files/source_file.hpp"
#include "fmt/format.h"

namespace scar {

    struct TextPos {
        size_t line = 1;
        size_t col = 1;
        size_t idx = 0;

        /*inline TextPos operator+(const TextPos& other) {
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
        }*/
    };

    struct Span {
        source::file_ptr_t file;
        size_t lo_line = 1, lo_col = 1;
        size_t hi_line = 1, hi_col = 1;
        size_t start = 0, len = 0;

        Span() = default;
        Span(source::file_ptr_t sf, const TextPos& lo, const TextPos& hi) :
            file(sf),
            lo_line(lo.line), lo_col(lo.col),
            hi_line(hi.line), hi_col(hi.col),
            start(lo.idx), len(hi.idx - lo.idx)
        {}
    };

    struct LabledSpan {
        Span span;
        std::string label;

        LabledSpan(Span sp, std::string label) :
            span(std::move(sp)),
            label(std::move(label))
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
            sp.lo_line, sp.lo_col,
            sp.hi_line, sp.hi_col);
    }
};