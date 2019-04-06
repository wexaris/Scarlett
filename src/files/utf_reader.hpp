#pragma once
#include "codepoint.hpp"
#include "source_file.hpp"

class UTFReader {

protected:
    SourceFile* sourcefile;

    size_t index = 0;

    char read_byte();

public:
    explicit UTFReader(const std::string& filepath);
    virtual ~UTFReader() {}

    Codepoint read();

    inline size_t get_index() const           { return index; }
    inline const SourceFile* get_sf() const { return sourcefile; }
};