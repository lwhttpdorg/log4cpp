include(FetchContent)

project(liblog4cpp)

set(TARGET_NAME log4cpp)

file(GLOB SRC_FILES "*.cpp")

if (CMAKE_HOST_WIN32)
    add_library(${TARGET_NAME} STATIC ${SRC_FILES})
else ()
    add_library(${TARGET_NAME} SHARED ${SRC_FILES})
endif ()

#find_package(Boost 1.75 REQUIRED COMPONENTS json)
#if (Boost_FOUND)
#    #message(STATUS "Boost_LIB_VERSION = ${Boost_VERSION}")
#    #message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
#    #message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
#    #message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
#    include_directories(${Boost_INCLUDE_DIRS})
#    link_directories(${Boost_LIBRARY_DIRS})
#    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
#endif ()

set(BOOST_INCLUDE_LIBRARIES json algorithm)
set(BOOST_ENABLE_CMAKE ON)
if ((CMAKE_HOST_WIN32) AND (ENABLE_TESTS))
    set(BUILD_SHARED_LIBS OFF)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()
set(BOOST_RUNTIME_LINK shared)
set(BOOST_INSTALL_LAYOUT system)
set(BOOST_ENABLE_PYTHON OFF)
set(BOOST_ENABLE_MPI OFF)
if (${CMAKE_VERSION} VERSION_LESS "3.24")
    FetchContent_Declare(
            Boost
            URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.zip
            URL_HASH SHA256=03530dec778bc1b85b070f0b077f3b01fd417133509bb19fe7c142e47777a87b
    )
else ()
    FetchContent_Declare(
            Boost
            URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.zip
            URL_HASH SHA256=03530dec778bc1b85b070f0b077f3b01fd417133509bb19fe7c142e47777a87b
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    )
endif ()

FetchContent_MakeAvailable(Boost)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set_target_properties(boost_json PROPERTIES COMPILE_FLAGS "-g -O0")
        set_target_properties(boost_algorithm PROPERTIES COMPILE_FLAGS "-g -O0")
    endif ()
else ()
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set_target_properties(boost_json PROPERTIES COMPILE_FLAGS "-g -O2")
        set_target_properties(boost_algorithm PROPERTIES COMPILE_FLAGS "-g -O2")
    endif ()
endif ()
target_link_libraries(${TARGET_NAME} Boost::json Boost::algorithm)

if (CMAKE_HOST_WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_definitions(WIN32_LEAN_AND_MEAN)
    endif ()
    target_link_libraries(${TARGET_NAME} ws2_32)
endif ()

if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
endif ()

if (ENABLE_ASAN)
    target_link_options(${TARGET_NAME} PRIVATE "-fsanitize=address -fsanitize=undefined")
endif ()