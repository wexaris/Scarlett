#pragma once
#include "cmd_args/args.hpp"
#include "front/sym_table.hpp"

namespace scar {

    class Session {

    private:
        shared<SymbolStack> sym_stack = std::make_shared<SymbolStack>();
        args::ParsedArgs args;

        Session() = default;
        Session(const Session&) = delete;

        inline void operator=(const Session&) = delete;

        /* Updates the systems that respond to command line arguments. */
        void apply_args();

    public:
        static Session& instance() {
            static Session session;
            return session;
        }

        static void init(int argc, const char** argv);

        inline const args::ParsedArgs& get_args() const noexcept { return args; }
        inline shared<SymbolStack> get_symbols() const noexcept { return sym_stack; }
    };

}