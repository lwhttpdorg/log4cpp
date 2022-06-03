cmake_minimum_required(VERSION 3.0)
project(demo)

set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if (Threads_FOUND)
    if (CMAKE_USE_PTHREADS_INIT)
        target_link_libraries(${TARGET_NAME} Threads::Threads)
    else ()
        message(FATAL_ERROR "pthread is required")
    endif ()
else ()
    message(FATAL_ERROR "pthread not found!")
endif ()

include_directories(${PROJECT_SOURCE_DIR}/../src)
message(STATUS ${PROJECT_SOURCE_DIR})
target_link_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(${TARGET_NAME} log4cpp)