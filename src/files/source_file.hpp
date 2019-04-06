#pragma once
#include "common.hpp"
#include <fstream>

struct SourceFile {

    std::string source_code;
public:
    const std::string name;

    explicit SourceFile(const std::string& path);
    virtual ~SourceFile() {}

    char operator[] (size_t idx) const { return get(idx); }

    /* Get the char in the source code at the given index. */
    char get(size_t idx) const;
    /* Get a view of some substring in the source code.
    The low index is inclusive, the high index is exclusive. */
    std::string_view get(size_t idx_lo, size_t idx_hi) const;

    inline size_t len() const { return source_code.length(); }

    inline const std::string& text() const      { return source_code; }
};