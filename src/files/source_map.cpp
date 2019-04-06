#include "source_map.hpp"

std::vector<unique<SourceFile>> SourceMap::files;

SourceFile* SourceMap::find(const std::string& filepath) {
    for (auto& file : files) {
        if (file->name == filepath) {
            return file.get();
        }
    }
    return nullptr;
}

SourceFile* SourceMap::load(const std::string& filepath) {
    files.push_back(new_unique(SourceFile, filepath));
    return files.back().get();
}

SourceFile* SourceMap::request(const std::string& filepath) {
    auto file = find(filepath);
    if (!file)
        file = load(filepath);
    return file;
}