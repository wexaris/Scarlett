cmake_minimum_required( VERSION 3.6 )
project( scarlet )

# lower case build type
# used for directory paths
string( TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE )

# set output directories
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYPE} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYPE} )

#
# set the compiler version
# should be C++17 for [[attributes]] to work
set( CMAKE_CXX_STANDARD 17 )

# set inlude path
include_directories(
    "src/"
    "fmt/include/"
)

# all of the source files to compile
set( SRC_FILES
    src/main.cpp
    src/cmd_args/args.cpp
    src/driver/driver.cpp
    src/front/parser.cpp
    src/front/interner.cpp
    src/front/token.cpp
    src/front/ast.cpp
    src/front/lex/lexer.cpp
    src/front/lex/utf_reader.cpp
    src/front/lex/codepoint.cpp
    src/front/files/source.cpp
    src/log/registry.cpp
    src/log/logger.cpp
)

# GNUCXX compile options
if( CMAKE_COMPILER_IS_GNUCXX )
    # set universal parameters
    add_compile_options( -pedantic -Wall -Wextra -Werror )

    # add debug / release
    if( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
        add_compile_options( -DDEBUG -g3 -O0 )
    else()
        add_compile_options( -O3 )
    endif()
endif()

add_subdirectory( fmt )

add_executable( scar ${SRC_FILES} )

target_link_libraries( scar fmt )