#include "driver/driver.hpp"
#include "util/early_exit.hpp"

int main(int argc, const char* argv[]) {

    try {
        scar::Session::get().init(argc, argv);

        scar::Driver driver;
        driver.run();
    }
    catch (const scar::early::FatalError& e) {
        return e.code;
    }
    catch (const scar::early::EarlyExit& e) {
        return e.code;
    }

    return 0;
}