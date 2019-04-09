#pragma once
#include "parse/ast.hpp"
#include "common.hpp"
#include <unordered_map>

class Interner {
    std::unordered_map<std::string, ast::Name> string_id_map;
    std::vector<InternedString> interned_strings;

public:
    explicit Interner() = default;
    explicit Interner(std::vector<std::string> strs);

    ast::Name intern(std::string str);

    ast::Name* find(const std::string& str);
    
    inline InternedString get(const ast::Name& name) const { return interned_strings[name.id]; }
};
