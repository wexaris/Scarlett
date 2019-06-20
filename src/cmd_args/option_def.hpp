#pragma once
#include "arg_tree.hpp"
#include <unordered_map>
#include <functional>

namespace scar {
    namespace args {

        class CMDLineArgIndexer {

        private:
            std::vector<std::string_view> args;
            int idx;
            int end;

        public:
            CMDLineArgIndexer(int argc, const char** argv, int start_idx);

            inline std::string_view get() const         { return args[idx]; }
            inline std::string_view get(int i) const    { return args[i]; }
            inline void next()                          { if (idx <= end) idx++; }

            inline bool is_end() const { return idx > end; }
        };

        class OptionDef {

        private:
            using callback_t = std::function<void(ArgTree&)>;
            using predef_t = std::unordered_map<size_t, std::pair<bool, std::vector<std::string_view>>>;

            std::string_view first;
            size_t sub_num;
            predef_t predef_subs;
            callback_t callback;

        public:
            OptionDef(std::string_view first, callback_t cb = nullptr);
            OptionDef(std::string_view first, size_t subparam_num, callback_t cb = nullptr);
            OptionDef(std::string_view first, std::vector<std::vector<std::string_view>> predef, callback_t cb = nullptr);

            std::optional<ArgTree> check(CMDLineArgIndexer& cmd) const;
        };

    }
}