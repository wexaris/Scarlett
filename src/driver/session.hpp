#pragma once
#include "cmd_args/args.hpp"
#include "symbols/sym_table.hpp"
#include "log/log_manager.hpp"

namespace scar {

    class Session {

    private:
        log::LogManager log_mgr;
        SymbolStack sym_stack;
        args::ParsedArgs cmd_args;

        Session() = default;
        Session(const Session&) = delete;

        inline void operator=(const Session&) = delete;

        /* Updates the systems that respond to command line arguments. */
        void apply_args();

    public:
        static Session& get() {
            static Session session;
            return session;
        }

        void init(int argc, const char** argv);

        inline SymbolStack& symbols() noexcept               { return sym_stack; }
        inline log::LogManager& logger() noexcept            { return log_mgr; }
        inline const args::ParsedArgs& args() const noexcept { return cmd_args; }
    };

}