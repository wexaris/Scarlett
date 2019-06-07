#include "driver/driver.hpp"
#include "util/early_exit.hpp"

int main(int argc, const char* argv[]) {

    try {
        scar::Session::init(argc, argv);

        scar::Driver driver;
        driver.run();
    }
    catch (const scar::FatalError& e) {
        return e.code;
    }
    catch (const scar::EarlyExit& e) {
        return e.code;
    }

    return 0;
}