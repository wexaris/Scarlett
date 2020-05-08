#include "scarpch.hpp"
#include "Core/Driver.hpp"

int main(int argc, const char* argv[]) {
 
    scar::Driver::Init(argc, argv);
    scar::Driver::Compile();

    return scar::Driver::GetReturnState();
}