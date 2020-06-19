#pragma once

int main(int argc, const char* argv[]);

namespace scar {

    class Driver {
    private:
        friend int ::main(int argc, const char* argv[]);

        static void Init(const std::vector<const char*>&);
        static void Compile();
        static void Exit();
    };

}