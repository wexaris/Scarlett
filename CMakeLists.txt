cmake_minimum_required(VERSION 3.6)
project(scarlett)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

### External
add_subdirectory(external)

find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(LLVM_LIBS support core irreader passes)
message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM config directory: ${LLVM_DIR}")

### Scarlett
set(SCAR_SRC
    src/main.cpp
    src/Core/Driver.cpp
    src/Core/Session.cpp
    src/Core/Log.cpp
    src/Parse/Parser.cpp
    src/Parse/Interner.cpp
    src/Parse/Token.cpp
    src/Parse/AST/LLVMVisitor.cpp
    src/Parse/AST/VerifyVisitor.cpp
    src/Parse/AST/PrintVisitor.cpp
    src/Parse/Lex/Lexer.cpp
    src/Parse/Lex/UTFReader.cpp
    src/Parse/Lex/SourceFile.cpp
    src/Parse/Lex/Codepoint.cpp
)

set(SCAR_PCH src/scarpch.hpp)

add_executable(scar ${SCAR_SRC})
target_precompile_headers(scar PRIVATE ${SCAR_PCH})

target_link_libraries(scar PUBLIC ${LLVM_LIBS} fmt spdlog)
target_compile_definitions(scar PUBLIC ${LLVM_DEFINITIONS})
target_include_directories(scar PUBLIC src/ SYSTEM ${LLVM_INCLUDE_DIRS})

target_compile_options(scar PRIVATE
    $<$<CONFIG:DEBUG>:   #DEBUG
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -pedantic -O0 -g -Wno-unused-function>
        $<$<CXX_COMPILER_ID:MSVC>:
            /ZI /W3 /RTC1 /Od>>
    $<$<CONFIG:RELEASE>: #RELEASE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -O2 -s -DNDEBUG>
        $<$<CXX_COMPILER_ID:MSVC>:
            /Zi /GL /O2>>
)