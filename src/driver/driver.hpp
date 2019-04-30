#pragma once
#include "session.hpp"
#include "front/ast.hpp"

namespace scar {

    class Driver {

    public:
        Driver() = default;
        ~Driver() = default;

        void run();
    };

}
