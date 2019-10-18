#pragma once
#include "files/source_file.hpp"
#include "fmt/format.h"

#define FMT(...) fmt::format(__VA_ARGS__)

namespace scar {

    struct TextPos {
        size_t line = 1;
        size_t col = 1;
        size_t idx = 0;
    };

    struct Span {
        source::file_ptr_t file;
        size_t line = 1, col = 1;
        size_t idx = 0, len = 0;

        Span() = default;
		Span(source::file_ptr_t sf, size_t line, size_t col, size_t idx, size_t len) :
			file(sf),
			line(line), col(col),
			idx(idx), len(len)
		{}
		Span(source::file_ptr_t sf, const TextPos& lo, const TextPos& hi) :
			Span(sf, lo.line, lo.col, lo.idx, hi.idx - lo.idx)
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
        return fmt::format_to(ctx.out(), "{}:{}:{}",
            sp.file->name(), sp.line, sp.col);
    }
};