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

            std::optional<std::vector<Argument>::iterator> find(std::string_view flag) {
                for (auto f_iter = flags.begin(); f_iter != flags.end(); f_iter++) {
                    if (*f_iter == flag)
                        return f_iter;
                }
                return std::nullopt;
            }

            inline bool contains(std::string_view flag) { return find(flag).has_value(); }
        };

        struct ArgTreeList {
            std::vector<ArgTree> flag_trees;

            ArgTreeList() = default;

            inline void add(ArgTree&& ft) {
                flag_trees.push_back(std::move(ft));
            }

            std::optional<std::vector<Argument>> find(std::string_view flag) const {
                for (auto& ft : flag_trees) {
                    if (*ft.flags.begin() == flag) {
                        return ft.flags;
                    }
                }
                return std::nullopt;
            }

            inline bool contains(std::string_view flag) const {
                return find(flag).has_value();
            }
        };

    }
}