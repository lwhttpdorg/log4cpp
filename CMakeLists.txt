cmake_minimum_required(VERSION 3.0)

project(log4cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -pipe")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -march=native -pipe")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Debug version...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else ()
    message(STATUS "Release version...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()

add_subdirectory(src)
