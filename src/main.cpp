#include "driver/driver.hpp"

int main(uint argc, const char* argv[]) {
    
    ProgramArgs args = compose_program_args(argc, argv);
    Driver driver = Driver(args);

    driver.compile();
    
    return EXIT_SUCCESS;
}