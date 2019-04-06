#include "source_file.hpp"
#include "source_map.hpp"

SourceFile::SourceFile(const std::string& path) :
    name(path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        ERR(FMT("Filed to open file at " << path));
    }

    // Reserve string size
    file.seekg(0, std::ios::end);
    source_code.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    source_code.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());

    file.close();
}

char SourceFile::get(size_t idx) const {
    return source_code[idx];
}

std::string_view SourceFile::get(size_t idx_lo, size_t idx_hi) const {
    return std::string_view(source_code).substr(idx_lo, idx_hi - idx_lo);
}
