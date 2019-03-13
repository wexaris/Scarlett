#include "source_file.hpp"
#include "source_map.hpp"

SourceFile::SourceFile(const std::string& path) :
    std::ifstream(path),
    filename(path)
{
    if (!is_open()) {
        ERR(FMT("Filed to open file at " << path));
    }
}

SourceFile::~SourceFile() {
    close();
}