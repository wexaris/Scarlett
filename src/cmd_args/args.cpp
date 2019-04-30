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

    }
}