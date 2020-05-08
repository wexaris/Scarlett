# Scarlett

This is the source code repository for the work-in-progress _scar_ compiler.


## Building from Source

1. To build the compiler you will need the following dependencies:
    * `git`
    * `make`
    * `cmake`
    * `g++`
    * `llvm`

2. Start off by cloning the source code from this repository:
    ```
    git clone https://github.com/wexaris/scarlet.git
    cd scarlet
    ``` 

3. After that you can just build the compiler by running one of
    ```
    make
    make debug
    make release
    ```
    > The default build type is set to Debug.

    The actual build system is CMake, but the Makefile is used as a shorthand for
    specifying build types, making directories, and cleaning up.