# Scarlet

This is the source code repository for the work-in-progress _scar_ compiler.

The project is meant to largely canibalize the _ivy_ compiler, since it was made 
as part of a school project and many of it's parts are rushed, incomplete or badly designed.
This project aims to rework and expand upon the ideas and experience accumulated in the
process of working on the _ivy_ project.

## Usage

Using the compiler is very simple. All you need to do is pass it some input files.
Other parameters the compiler recognises can be found in the help menu `-h`.

```
scar [options] <input>
```
> Some of the options currently have no effect since the compiler does not yet
produce machine code.


## Building from Source

1. To build the compiler you will need the following dependencies:
    * `git`
    * `make`
    * `cmake`
    * `g++`
    * `fmt`
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