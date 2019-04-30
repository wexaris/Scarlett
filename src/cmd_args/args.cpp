#include "args.hpp"
#include "option_def.hpp"
#include "settings.hpp"

namespace scar {
    namespace args {

        inline void print_version() {
            log::get_default()->info("scar {}", version::get_version());
        }

        inline void print_usage() {
            log::get_default()->info(
                "USAGE: scar [options] <input>\n"
                "\n"
                "OPTIONS:\n"
                "    -h             display this help menu\n"
                "    -o <...>       write the output file to the given location\n"
                "    -g             include debug information\n"
                "    -O             enable program optimization\n"
                "    -Woff          disable compiler warnings\n"
                "    -Werr          treat warnings as errors\n"
                "\n"
                "    --name <...>   set output file name\n"
            );
        }

        static std::vector<OptionDef> option_defs{
            { "-h",         [](ArgTree&) { print_usage(); throw EarlyExit(0); } },
            { "--version",  [](ArgTree&) { print_version(); throw EarlyExit(0); } },
            { "-o", 1 },
            { "-g" },
            { "-O" },
            { "-Woff" },
            { "-Werr" },
            { "--name", 1 }
        };

        const ParsedArgs parse(int argc, const char* argv[]) {

            // Missing arguments
            if (argc == 1) {
                print_usage();
                throw EarlyExit(0);
            }

            std::vector<std::string_view> ifiles;
            std::string_view ofile;
            ArgTreeList atlist;

            CMDLineArgIndexer cmd(argc, argv, 1);

            while (!cmd.is_end()) {
                // Check if arg doesn't start with a dash,
                // meaning it's anm input file
                auto arg = cmd.get();
                if (arg[0] != '-') {
                    ifiles.push_back(arg);
                    cmd.next();
                    continue;
                }

                for (auto& def : option_defs) {
                    // Check if the current args match an option definition
                    auto ft_opt = def.check(cmd);
                    if (ft_opt.has_value()) {
                        atlist.add(std::move(ft_opt.value()));
                        goto next_arg_loop;
                    }
                }

                log::get_default()->critical("unrecognized option '{}'", arg);
                throw EarlyExit(1);

            next_arg_loop:
                cmd.next();
            }

            // Check for missing input files
            if (ifiles.size() == 0) {
                log::get_default()->error("missing input files");
                throw EarlyExit(0);
            }

            // Separate output file parameter
            auto ofile_opt = atlist.find("-o");
            ofile = ofile_opt.has_value() ? ofile_opt->back() : "out";

            return ParsedArgs{ ifiles, ofile, atlist };
        }

        OptionDef::OptionDef(std::string_view first, callback_t cb) :
            OptionDef(first, 0, cb)
        {}
        OptionDef::OptionDef(std::string_view first, size_t subparam_num, callback_t cb) :
            first(first),
            sub_num(subparam_num),
            callback(cb)
        {}
        OptionDef::OptionDef(std::string_view first, std::vector<std::vector<std::string_view>> predef, callback_t cb) :
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

        std::optional<ArgTree> OptionDef::check(CMDLineArgIndexer& cmd) const {
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


        std::optional<std::vector<Argument>::iterator> ArgTree::find(std::string_view flag) {
            for (auto f_iter = flags.begin(); f_iter != flags.end(); f_iter++) {
                if (*f_iter == flag)
                    return f_iter;
            }
            return std::nullopt;
        }

        std::optional<std::vector<Argument>> ArgTreeList::find(std::string_view flag) const {
            for (auto& ft : flag_trees) {
                if (*ft.flags.begin() == flag) {
                    return ft.flags;
                }
            }
            return std::nullopt;
        }


        CMDLineArgIndexer::CMDLineArgIndexer(int argc, const char** argv, int start_idx) :
            idx(start_idx),
            end(argc - 1)
        {
            for (int i = 0; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }

    }
}