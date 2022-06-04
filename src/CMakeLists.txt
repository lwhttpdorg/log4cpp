cmake_minimum_required(VERSION 3.18)
project(log4cpp)

set(TARGET_NAME log4cpp)

set(CMAKE_CXX_STANDARD 11)

add_library(${TARGET_NAME} SHARED log4cpp.cpp)

if (NOT CMAKE_HOST_UNIX)
    message(FATAL_ERROR "Only support Linux/Unix!")
endif ()