#pragma once
#include "common.hpp"

using ProgramArgs = std::vector<std::string>;
ProgramArgs compose_program_args(unsigned argc, const char* argv[]);

struct Session {

    std::string infile;
    std::string outfile;

    std::string package_name;

	unsigned optimize_lvl = 0;
	bool include_debug_info = false;

    enum OutputType {
        BIN,
        LIB
    } output_type = BIN;

public:
    Session(const ProgramArgs& args);

    void show_help() const;
};