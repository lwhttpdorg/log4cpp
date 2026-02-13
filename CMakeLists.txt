cmake_minimum_required(VERSION 3.10)

project(log4cpp VERSION 4.0.5 LANGUAGES CXX)

include(GNUInstallDirs)

# Build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif ()
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()

# Options
option(BUILD_LOG4CPP_DEMO "Build demo application" OFF)
option(ENABLE_LOG4CPP_UNIT_TEST "Enable log4cpp unit tests" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_LOG4CPP_COVERAGE "Enable log4cpp code coverage (GNU only)" OFF)

# nlohmann_json
find_package(nlohmann_json 3.0 REQUIRED)

# Helper: apply ASAN to a target (GNU/Clang only)
function(apply_asan_to_target tgt)
    if (ENABLE_ASAN)
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${tgt} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
                -fno-sanitize-recover=all
                -fsanitize=float-divide-by-zero
                -fsanitize=float-cast-overflow
                -fno-sanitize=null
                -fno-sanitize=alignment
            )
            if (CMAKE_BUILD_TYPE MATCHES "Debug")
                target_compile_options(${tgt} PRIVATE -fno-omit-frame-pointer)
            endif ()
            target_link_options(${tgt} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
            )
        elseif (MSVC)
            message(WARNING "MSVC AddressSanitizer is not supported in this configuration; ENABLE_ASAN is ignored.")
        endif ()
    endif ()
endfunction()

if (ENABLE_LOG4CPP_COVERAGE)
    # Code coverage target for GNU builds
    add_library(code_coverage INTERFACE)
    target_compile_options(code_coverage INTERFACE --coverage -O0 -g)
    target_link_options(code_coverage INTERFACE --coverage)
endif ()

# Library
add_subdirectory(src)

# Demo
if (BUILD_LOG4CPP_DEMO)
    add_subdirectory(demo)
endif ()

# Tests
if (ENABLE_LOG4CPP_UNIT_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/log4cpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/log4cpp.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/log4cpp.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
