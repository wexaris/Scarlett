#pragma once
#include <optional>
#include <vector>
#include <string>

namespace scar {
    namespace args {

        using Argument = std::string_view;

        struct ArgTree {
            std::vector<Argument> flags;

            ArgTree() = default;

            inline void add(Argument flag) {
                flags.push_back(std::move(flag));
            }

            std::optional<std::vector<Argument>::iterator> find(std::string_view flag);

            inline bool contains(std::string_view flag) {
                return find(flag).has_value();
            }
        };

        struct ArgTreeList {
            std::vector<ArgTree> flag_trees;

            ArgTreeList() = default;

            inline void add(ArgTree&& ft) {
                flag_trees.push_back(std::move(ft));
            }

            std::optional<std::vector<Argument>> find(std::string_view flag) const;

            inline bool contains(std::string_view flag) const {
                return find(flag).has_value();
            }
        };

    }
}