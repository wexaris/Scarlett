#pragma once
#include "arg_tree.hpp"

namespace scar {
    namespace args {

        struct ParsedArgs {
            std::vector<std::string_view> in;
            std::string_view out;
            ArgTreeList args;
        };

        const ParsedArgs parse(int argc, const char* argv[]);
    }
}