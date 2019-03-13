#pragma once
#include "common.hpp"
#include <fstream>

struct SourceFile : public std::ifstream {
    friend class SourceMap;

private:
    bool available = true;

public:
    const std::string filename;

    explicit SourceFile(const std::string& path);
    virtual ~SourceFile();

    inline void free()          { available = false; }
    inline bool is_free() const { return this->available; }
};