#pragma once
#include "common.hpp"
#include "parse/ast.hpp"
#include <unordered_map>

class Interner {
    std::unordered_map<std::string, ast::Name> string_id_map;
    std::vector<shared<std::string>> interned_strings;

public:
    explicit Interner() = default;
    explicit Interner(std::vector<std::string> strs);

    ast::Name intern(std::string str);

    std::string get(const ast::Name& name) { return *interned_strings[name.id]; }

    ast::Name* find(const std::string& str);
};