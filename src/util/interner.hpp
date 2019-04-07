#pragma once
#include "common.hpp"
#include "parse/ast.hpp"
#include <unordered_map>

class Interner {
    std::unordered_map<std::string, ast::Name> names;
    std::vector<shared<std::string>> strings;

public:
    explicit Interner() = default;
    explicit Interner(std::vector<std::string> strs)
    {
        strings.reserve(strs.size());
        for (const auto& s : strs) {
            intern(s);
        }
    }

    ast::Name intern(std::string str) {
        ast::Name name{ strings.size() };

        strings.push_back(mk_shared(str));
        names[str] = name;

        return name;
    }

    inline std::string get(const ast::Name& name) {
        return *strings[name.id];
    }

    inline ast::Name* find(const std::string& str) {
        auto name = names.find(str);
        if (name != names.end()) {
            return &name->second;
        }
        return nullptr;
    }
};