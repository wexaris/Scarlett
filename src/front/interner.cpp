#include "interner.hpp"

namespace scar {

    ast::Name Interner::intern(std::string str) {
        auto find_str = find(str);
        if (find_str.has_value()) {
            return find_str.value();
        }

        auto name = ast::Name{ interned_strings.size() };

        string_idx_map[str] = name;
        interned_strings.push_back(std::make_shared<std::string>(str));

        return name;
    }

    std::optional<ast::Name> Interner::find(const std::string& str) const {
        auto name = string_idx_map.find(str);
        if (name != string_idx_map.end()) {
            return name->second;
        }
        return std::nullopt;
    }

}