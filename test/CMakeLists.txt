# Force shared CRT on Windows for GoogleTest
set(gtest_force_shared_crt ON)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# Collect JSON config files from test/config/
file(GLOB TEST_CONFIG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/*.json")

# Define an INTERFACE library for common test includes
add_library(test_includes INTERFACE)
target_include_directories(test_includes INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include # For test-specific headers like log4cpp_test.h
        ${CMAKE_SOURCE_DIR}/src/include      # For internal library headers like common/log_utils.hpp
)

# Define a function to simplify adding tests
function(add_log4cpp_test test_name)
    add_executable(${test_name} "app/${test_name}.cpp")

    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

    # Link against GoogleTest, main library, and common includes
    target_link_libraries(${test_name} PRIVATE GTest::gtest_main log4cpp test_includes)
    if (test_name STREQUAL "load_config_test" OR test_name STREQUAL "serialize_test")
        target_link_libraries(${test_name} PRIVATE nlohmann_json::nlohmann_json)
    endif ()

    if (CMAKE_HOST_WIN32)
        target_link_libraries(${test_name} PRIVATE ws2_32)
    endif ()

    # Copy config files needed for tests using a post-build command
    if (TEST_CONFIG_FILES)
        # Copy to the binary directory for IDE runners (e.g., CLion)
        add_custom_command(TARGET ${test_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TEST_CONFIG_FILES} $<TARGET_FILE_DIR:${test_name}>
                COMMENT "Copying test configs to bin directory for ${test_name}"
        )
        # Copy to the CTest working directory
        add_custom_command(TARGET ${test_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TEST_CONFIG_FILES} ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Copying test configs to CTest directory for ${test_name}"
        )
    endif ()

    # Apply AddressSanitizer if enabled
    apply_asan_to_target(${test_name})

    # Register with CTest using GoogleTest discovery
    include(GoogleTest)
    gtest_discover_tests(${test_name} DISCOVERY_TIMEOUT 30 PROPERTIES TIMEOUT 30)

    # Check if the generator is MSVC (Visual Studio)
    if (MSVC)
        # Define _CRT_SECURE_NO_WARNINGS universally for MSVC to suppress C4996 warnings
        target_compile_definitions(${test_name} PRIVATE _CRT_SECURE_NO_WARNINGS)

        # Conditionally define the architecture macros based on the target platform size
        # CMAKE_SIZEOF_VOID_P is 8 for 64-bit (x64/ARM64) and 4 for 32-bit (x86/ARM)
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            # 64-bit architecture (x64)
            target_compile_definitions(${test_name} PRIVATE _AMD64_)
        elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            # 32-bit architecture (x86)
            target_compile_definitions(${test_name} PRIVATE _X86_)
        endif ()
    endif ()
endfunction()

# Add all test targets
add_log4cpp_test(load_config_test)
add_log4cpp_test(log_pattern_test)
add_log4cpp_test(console_appender_test)
add_log4cpp_test(file_appender_test)
add_log4cpp_test(socket_appender_test)
add_log4cpp_test(serialize_test)
# This test is not supported on Windows
if (NOT WIN32)
    add_log4cpp_test(config_hot_reload_test)
endif ()
