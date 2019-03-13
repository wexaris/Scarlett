#pragma once
#include "session.hpp"

/* The backbone of the compilation process.
Takes care of handling all the other smaller parts of compilation. */
class Driver {

    /* The settings pertaining to the current compilation.
    Includes optimisation and debugging options, input and ouput file locations,
    information about the package, etc. */
    shared<Session> sess;

public:
    /* Create Driver with a pre-existing Session. */
    explicit Driver(std::shared_ptr<Session> sess)
        : sess(sess)
    {}

    /* Create Driver given some program arguments.
    A Session will be created internally. */
    explicit Driver(const ProgramArgs& args)
        : sess(new_shared(Session, args))
    {}

    /* Begin the compilation process. */
    void compile();

    const std::shared_ptr<Session> get_session() const { return sess; }
};
