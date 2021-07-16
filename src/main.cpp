#include "scarpch.hpp"
#include "Core/Driver.hpp"
#include "Core/Session.hpp"

std::vector<const char*> VectorizeArgs(int argc, const char* argv[]) {
    std::vector<const char*> args;
    args.reserve(argc);
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    return args;
}

int main(int argc, const char* argv[]) {

    scar::Driver::Init(VectorizeArgs(argc, argv));
    scar::Driver::Compile();
    scar::Driver::Exit();

    return scar::Session::IsGood() ? 0 : -1;
}