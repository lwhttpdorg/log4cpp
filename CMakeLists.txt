cmake_minimum_required(VERSION 3.18)
project(log4cpp)

set(CMAKE_CXX_STANDARD 20)

add_library(log4cpp SHARED src/log4cpp.cpp src/utils.cpp)
