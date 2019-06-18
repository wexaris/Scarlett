cmake_minimum_required( VERSION 3.6 )
project( scarlet LANGUAGES CXX VERSION 0.0.1 )

####################
####################

# set default build type if none was specified
set( DEFAULT_BUILD_TYPE Release )
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
    message( STATUS "Set build type to '${DEFAULT_BUILD_TYPE}' as none was specified." )
  
    set( CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
        STRING "Choose build type" FORCE )

    # list possible build types for cmake-gui
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo" )
endif()

if( CMAKE_BUILD_TYPE )
    message( STATUS "Build: ${CMAKE_BUILD_TYPE}" )
endif()

# set output directories
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


####################
####################

# set verion to C++17
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )
message( STATUS "CXX verison: ${CMAKE_CXX_STANDARD}" )

# GNUCXX compile options
if( CMAKE_COMPILER_IS_GNUCXX )
    message( STATUS "Found GNUCXX compiler" )
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -g3 -O0 -pedantic -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif ( MSVC )
    message( STATUS "Found MSVC compiler" )
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

####################
####################

find_package( fmt REQUIRED CONFIG )

message( STATUS "FMT version: ${fmt_VERSION}" )
message( STATUS "FMT config location: ${fmt_DIR}" )

####################
####################

find_package( LLVM REQUIRED CONFIG )

message( STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}" )
message( STATUS "LLVM config location: ${LLVM_DIR}" )

include_directories( ${LLVM_INCLUDE_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )

llvm_map_components_to_libnames( llvm_libs support core irreader )

####################
####################

include_directories(
    "src/"
)

set( SRC_FILES
    src/main.cpp
    src/ast/ast.cpp
    src/ast/visitor/llvm_visitor.cpp
    src/cmd_args/args.cpp
    src/driver/driver.cpp
    src/files/source.cpp
    src/log/registry.cpp
    src/log/logger.cpp
    src/log/format/default_fmt.cpp
    src/parse/error/error.cpp
    src/parse/lex/codepoint.cpp
    src/parse/lex/lexer.cpp
    src/parse/lex/utf_reader.cpp
    src/parse/parser.cpp
    src/parse/token.cpp
    src/symbols/interner.cpp
)

add_executable( scar ${SRC_FILES} )

target_link_libraries( scar fmt::fmt ${llvm_libs} )