#include "interner.hpp"

Interner::Interner(std::vector<std::string> strs) {
    interned_strings.reserve(strs.size());
    for (const auto& s : strs) {
        intern(s);
    }
}

ast::Name Interner::intern(std::string str) {
    ast::Name name{ interned_strings.size() };

    string_id_map[str] = name;
    interned_strings.push_back(share(str));

    return name;
}

ast::Name* Interner::find(const std::string& str) {
    auto name = string_id_map.find(str);
    if (name != string_id_map.end()) {
        return &name->second;
    }
    return nullptr;
}
