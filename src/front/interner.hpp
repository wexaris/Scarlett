#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <optional>
#include <memory>
#include <string>
#include <vector>

namespace scar {

    using interned_str_t = std::shared_ptr<std::string>;

    class Interner {

    private:
        std::unordered_map<std::string, ast::Name> string_idx_map;
        std::vector<interned_str_t> interned_strings;

        Interner();
        Interner(const Interner&) = delete;
        
        void operator=(const Interner&) = delete;

    public:
        static Interner& instance() {
            static Interner interner;
            return interner;
        }

        ast::Name intern(std::string str);
        std::optional<ast::Name> find(const std::string& str) const;
    };

}