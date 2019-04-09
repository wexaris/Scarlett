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
include_directories( src/ )

# all of the source files to compile
set( SRC_FILES
    src/main.cpp
    src/span.cpp
    src/global.cpp
    src/driver/driver.cpp
    src/driver/session.cpp
    src/parse/parser.cpp
    src/parse/lex/lexer.cpp
    src/parse/lex/bump_reader.cpp
    src/parse/lex/token.cpp
    src/files/source_map.cpp
    src/files/source_file.cpp
    src/files/utf_reader.cpp
    src/files/codepoint.cpp
    src/errors/errors.cpp
    src/util/interner.cpp
)

# GNUCXX compile options
if( CMAKE_COMPILER_IS_GNUCXX )
    message( "\n-- Setting GNU C/C++ compile options" )

    # set universal parameters
    add_compile_options( -pedantic -Wall -Wextra -Werror )

    # add debug / release
    if( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
        add_compile_options( -DDEBUG -g3 -O0 )
    else()
        add_compile_options( -O3 )
    endif()
endif()

add_executable( scar ${SRC_FILES} )