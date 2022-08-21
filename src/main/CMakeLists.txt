cmake_minimum_required(VERSION 3.18)
project(liblog4cpp)

set(TARGET_NAME log4cpp)

add_library(${TARGET_NAME} STATIC log4cpp.cpp LogConfiger.cpp)

if (NOT CMAKE_HOST_UNIX)
    message(FATAL_ERROR "Only support Linux/Unix!")
endif ()

set(INSTALL_HEAD_FILES log4cpp.hpp)
install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

include(FetchContent)
FetchContent_Declare(
        yaml_cpp
        GIT_REPOSITORY https://github.com/YukPingFong/yaml-cpp.git
        GIT_TAG d84e589c73f29f882eb514f6cb7f0d63bdbd517d
)

FetchContent_MakeAvailable(yaml_cpp)

target_link_libraries(${TARGET_NAME} yaml-cpp)

if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
endif ()
