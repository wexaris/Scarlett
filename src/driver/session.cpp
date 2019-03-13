#include "session.hpp"
#include "util/version.hpp"
#include <cstring>

#define OPTION_REQUIRES_ARG(x)  ERR(FMT("Option '" << x << "' requires an argument"))
#define UNRECOGNISED_VALUE(x)   ERR(FMT("Unrecognised value for '" << x << "'"))

ProgramArgs compose_program_args(unsigned argc, const char* argv[]) {
	ProgramArgs program_args = ProgramArgs(argc - 1);
	for (unsigned i = 1; i < argc; i++) {
        program_args[i - 1] = argv[i];
	}
	return program_args;
}

Session::Session(const ProgramArgs& args) {
	size_t argc = args.size();

	for (int i = 0; i < argc; i++) {

        std::string arg = args[i];

        // Argument without a '-' is assumed to be the input file
        // We only allow one input file per compile session
        if (arg[0] != '-') {
			if (!infile.empty()) {
				ERR("Only one input file is allowed");
			}
            infile = arg;
        }
        // "-o <file>" : Set output file
        else if (arg == "-o") {
			if (i == argc - 1) {
				OPTION_REQUIRES_ARG("-o");
			}
            outfile = args[++i];
        }
		// "-O" : Enable aplication optimization
		else if (arg == "-O") {
			optimize_lvl = 1;
		}
		// "-g" : Include debug information
		else if (arg == "-g") {
			include_debug_info = true;
		}
        // "-W <flag>" : Modify warning settings
        else if (arg == "-W") {
			if (i == argc - 1) {
				OPTION_REQUIRES_ARG("-W");
			}
            std::string flag = args[++i];

            if (flag == "enable") {
                // FIXME: enable warnings
            }
            else if (flag == "disable") {
                // FIXME: disable warnings
            }
            else if (flag == "err") {
                // FIXME: enable treating warnings as errors
            }
			else {
				UNRECOGNISED_VALUE("--out-type");
			}
        }
        // "-h" "--help" : Show help menu
        else if (arg == "-h" || arg == "--help") {
            show_help();
            exit(0);
        }
        // "--version" : Print version info
        else if (arg == "--version") {
            LOG("scar " << version_str());
            exit(0);
        }
        // --name <name> : Set package name
        else if (arg == "--name") {
			if (i == argc - 1) {
				OPTION_REQUIRES_ARG("--name");
			}
            package_name = args[++i];
        }
        // `--out-type <name>` : Set output type (bin | lib)
        else if (arg == "--out-type") {
			if (i == argc - 1) {
				OPTION_REQUIRES_ARG("--out-type");
			}
            std::string type = args[++i];

            if (type == "bin") {
                output_type = OutputType::BIN;
            }
            else if (type == "lib") {
                output_type = OutputType::LIB;
            }
			else {
				UNRECOGNISED_VALUE("--out-type");
			}
        }
		else {
			ERR(FMT("Unknown option: " << arg << "'"));
		}
    }
}

void Session::show_help() const {
    LOG(
        "USAGE: scar [options] <input>\n"
        "\n"
        "OPTIONS:\n"
        "    -h             display this help menu\n"
		"    -o <file>      write the output file to the given location\n"
		"    -O             enable program optimisation\n"
		"    -g             include debug information\n"
        "    -W <flag>\n"
        "       enable      enable compiler warnings\n"
        "       disable     disable compiler warnings\n"
        "       err         treat warnings as errors\n"
    );
}