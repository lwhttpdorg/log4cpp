cmake_minimum_required(VERSION 3.18)
project(liblog4cpp)

set(TARGET_NAME log4cpp)

add_library(${TARGET_NAME} STATIC log4cpp.cpp LogConfiger.cpp)

if (CMAKE_HOST_UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -Wextra -Wshadow -Wpointer-arith -Wwrite-strings \
	-Woverloaded-virtual \
	-Wno-pessimizing-move \
	-Wno-uninitialized -Wno-unused-parameter -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
	-Wno-unused-value -pipe")
endif ()

set(INSTALL_HEAD_FILES ${INCLUDE_DIRECTORIES}/log4cpp.hpp)
install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

include(FetchContent)
FetchContent_Declare(
        yaml_cpp
        GIT_REPOSITORY https://github.com/YukPingFong/yaml-cpp.git
        GIT_TAG cedc0aebf8be192d2311d1fe74e1bc202b9dc240
)

FetchContent_MakeAvailable(yaml_cpp)

if ((CMAKE_HOST_WIN32) AND (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
    target_link_libraries(${TARGET_NAME} yaml-cpp)
else ()
    target_link_libraries(${TARGET_NAME} yaml-cpp)
endif ()

if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
endif ()
