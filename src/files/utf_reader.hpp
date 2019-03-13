#pragma once
#include "codepoint.hpp"
#include "source_file.hpp"

class UTFReader {

protected:
    SourceFile* sourcefile;

    uint index = 0;

    char read_byte();

public:
    explicit UTFReader(const std::string& filepath);
    virtual ~UTFReader() {
        free_sf();
    }

    Codepoint read();

    inline uint get_index() const           { return index; }
    inline const SourceFile* get_sf() const { return sourcefile; }

    /* */
    inline void free_sf() { sourcefile->free(); }
};