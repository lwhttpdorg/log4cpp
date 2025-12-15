# Sources
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp")

add_library(log4cpp SHARED ${SRC_FILES})

# MSVC export symbols)
if (MSVC)
    set_target_properties(log4cpp PROPERTIES GENERATE_EXPORT_HEADER ON WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

# C++ standard scoped to target
target_compile_features(log4cpp PUBLIC cxx_std_17)

if (WIN32)
    target_compile_definitions(log4cpp PRIVATE UNICODE _UNICODE)
endif ()

# Check if the generator is MSVC (Visual Studio)
if (MSVC)
    # Define _CRT_SECURE_NO_WARNINGS universally for MSVC to suppress C4996 warnings
    target_compile_definitions(log4cpp PRIVATE _CRT_SECURE_NO_WARNINGS)

    # Conditionally define the architecture macros based on the target platform size
    # CMAKE_SIZEOF_VOID_P is 8 for 64-bit (x64/ARM64) and 4 for 32-bit (x86/ARM)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit architecture (x64)
        target_compile_definitions(log4cpp PRIVATE _AMD64_)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32-bit architecture (x86)
        target_compile_definitions(log4cpp PRIVATE _X86_)
    endif ()
endif ()

# Includes
target_include_directories(log4cpp
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform specifics
if (WIN32)
    if (MSVC)
        target_compile_definitions(log4cpp PRIVATE WIN32_LEAN_AND_MEAN)
    endif ()
    target_link_libraries(log4cpp PRIVATE ws2_32)
elseif (UNIX)
    target_link_libraries(log4cpp PRIVATE pthread)
endif ()

# GNU warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(log4cpp PRIVATE
        -Wall -Wold-style-cast -Wextra -Wshadow -Wpointer-arith -Wwrite-strings
        -Woverloaded-virtual -Wstringop-overflow -Wno-pessimizing-move
        -Wno-uninitialized -Wno-unused-parameter -Wno-unused-function
        -Wno-unused-variable -Wno-unused-value -pipe
    )
endif ()

target_link_libraries(log4cpp PUBLIC nlohmann_json::nlohmann_json)

# Output dirs
set_target_properties(log4cpp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# Apply ASAN
apply_asan_to_target(log4cpp)

# Install rules
install(TARGETS log4cpp
    EXPORT log4cppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/log4cpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
