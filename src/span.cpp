#include "span.hpp"
#include "files/source_file.hpp"

std::ostream& operator<<(std::ostream& os, const TextPos& sp) {
    os << sp.line << ":" << sp.col;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Span& sp) {
    os << sp.source->name << ":" << sp.lo << "-" << sp.hi;
    return os;
}