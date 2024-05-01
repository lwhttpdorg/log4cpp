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
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.zip
        URL_HASH SHA256=b5486e7b75715d5b1b2ef405a4cc29506d1a432a64e8b1bb8028df57239e768d
)
FetchContent_MakeAvailable(Boost)
target_link_libraries(${TARGET_NAME} Boost::json Boost::algorithm)

if (CMAKE_HOST_WIN32)
    target_link_libraries(${TARGET_NAME} ws2_32)
endif ()

if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
    if (ENABLE_ASAN)
        target_link_libraries(${TARGET_NAME} asan)
    endif ()
endif ()
