cmake_minimum_required(VERSION 3.6)
include(ExternalProject)

if(WIN32)
  set(prefix "")
  set(suffix ".lib")
else()
  set(prefix "lib")
  set(suffix ".a")
endif()

include(fmt.cmake)
include(spdlog.cmake)
