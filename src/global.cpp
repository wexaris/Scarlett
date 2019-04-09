#include "util/interner.hpp"

namespace global {

    thread_local Interner* interner = new Interner();

    void free() {
        delete interner;
    }

}
