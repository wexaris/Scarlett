#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <string>
#include <optional>
#include <memory>

namespace scar {

    using interned_str_t = std::shared_ptr<std::string>;

    class Interner {

    private:
        std::unordered_map<std::string, ast::Name> string_idx_map;
        std::vector<interned_str_t> interned_strings;

        Interner() = default;
        Interner(const Interner&) = delete;
        
        void operator=(const Interner&) = delete;

    public:
        static Interner& instance() {
            static Interner interner;
            return interner;
        }

        ast::Name intern(std::string str) {
            auto find_str = find(str);
            if (find_str.has_value()) {
                return find_str.value();
            }

            auto name = ast::Name{ interned_strings.size() };

            string_idx_map[str] = name;
            interned_strings.push_back(std::make_shared<std::string>(str));

            return name;
        }

        std::optional<ast::Name> find(const std::string& str) const {
            auto name = string_idx_map.find(str);
            if (name != string_idx_map.end()) {
                return name->second;
            }
            return std::nullopt;
        }
    };

}