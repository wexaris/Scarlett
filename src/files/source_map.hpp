#pragma once
#include "source_file.hpp"

class SourceMap {
    friend struct SourceFile;

    static std::vector<unique<SourceFile>> files;

    static SourceFile* find(const std::string& filepath);
    static SourceFile* find_free(const std::string& filepath);
    static SourceFile* load(const std::string& filepath);

public:
    SourceMap() = delete;
    ~SourceMap() = delete;

    static SourceFile* request(const std::string& filepath);
};