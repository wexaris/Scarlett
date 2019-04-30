#pragma once
#include "arg_tree.hpp"
#include "log/logging.hpp"
#include "util/early_exit.hpp"
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
            CMDLineArgIndexer(int argc, const char** argv, int start_idx) :
                idx(start_idx),
                end(argc - 1)
            {
                for (int i = 0; i < argc; i++) {
                    args.push_back(argv[i]);
                }
            }

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
            OptionDef(std::string_view first, callback_t cb = nullptr) :
                OptionDef(first, 0, cb)
            {}
            OptionDef(std::string_view first, size_t subparam_num, callback_t cb = nullptr) :
                first(first),
                sub_num(subparam_num),
                callback(cb)
            {}
            OptionDef(std::string_view first, std::vector<std::vector<std::string_view>> predef, callback_t cb = nullptr) :
                first(first),
                sub_num(predef.size()),
                callback(cb)
            {
                for (size_t i = 0; i < sub_num; i++) {
                    if (predef[i].size() != 0) {
                        bool allow_diff = predef[i][0] == "%other";
                        if (allow_diff) {
                            predef[i].erase(predef[i].begin());
                        }
                        predef_subs[i] = { allow_diff, std::move(predef[i]) };
                    }
                }
            }

            std::optional<ArgTree> check(CMDLineArgIndexer& cmd) const {
                ArgTree at;

                if (cmd.is_end()) {
                    return std::nullopt;
                }

                // Value won't be nullopt since we checked 'is_end()'
                auto arg = cmd.get();
                if (arg != first) {
                    return std::nullopt;
                }
                at.add(arg);

                for (size_t i = 0; i < sub_num; i++) {
                    cmd.next();

                    if (cmd.is_end() || cmd.get()[0] == '-') {
                        log::get_default()->error("missing parameter after '{}'", first);
                        throw EarlyExit(1);
                    }

                    auto predef_iter = predef_subs.find(i);
                    if (predef_iter != predef_subs.end()) {
                        auto arg = cmd.get();
                        for (auto& predef : (*predef_iter).second.second) {
                            if (arg == predef) {
                                at.add(arg);
                                goto next_sub_loop;
                            }
                        }
                        bool allow_diff = (*predef_iter).second.first;
                        if (!allow_diff) {
                            log::get_default()->error("invalid parameter '{}'", arg);
                            throw EarlyExit(1);
                        }
                    }
                    else {
                        at.add(cmd.get());
                    }

                next_sub_loop:;
                }

                if (callback != nullptr) {
                    callback(at);
                }

                return at;
            }
        };

    }
}