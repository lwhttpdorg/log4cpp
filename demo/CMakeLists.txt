cmake_minimum_required(VERSION 3.0)
project(demo)

set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.cpp)

find_package(Threads REQUIRED)
if (Threads_FOUND)
    target_link_libraries(${TARGET_NAME} Threads::Threads)
else ()
    message(FATAL_ERROR "pthread not found!")
endif ()

include_directories(${CMAKE_SOURCE_DIR}/src)
target_link_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(${TARGET_NAME} log4cpp)

if (INSTALL_DEMO)
    install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif ()