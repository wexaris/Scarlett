#pragma once
#include "cmd_args/args.hpp"

namespace scar {

    class Session {

    private:
        Session() = default;
        Session(const Session&) = delete;

        inline void operator=(const Session&) = delete;

        /* Updates the systems that respond to command line arguments. */
        void apply_args();

    public:
        args::ParsedArgs args;

        static Session& instance() {
            static Session session;
            return session;
        }

        static void init(int argc, const char** argv) {
            instance().args = scar::args::parse(argc, argv);
            instance().apply_args();
        }
    };

}