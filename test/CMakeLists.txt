# Force shared CRT on Windows for GoogleTest
set(gtest_force_shared_crt ON)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

# List of test targets
set(TEST_TARGETS
        load_config_tests
        log_pattern_tests
        console_appender_tests
        file_appender_tests
        socket_appender_tests
        serialize_test
        config_hot_reload_tests
)

# Map each test target to its source file
set(load_config_tests_SRC app/load_config_test.cpp)
set(log_pattern_tests_SRC app/log_pattern_test.cpp)
set(console_appender_tests_SRC app/console_appender_test.cpp)
set(file_appender_tests_SRC app/file_appender_test.cpp)
set(socket_appender_tests_SRC app/socket_appender_test.cpp)
set(serialize_test_SRC app/serialize_test.cpp)
set(config_hot_reload_tests_SRC app/config_hot_reload_test.cpp)

# Collect JSON config files from test/config/
file(GLOB TEST_CONFIG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/*.json")

# Define an INTERFACE library for common test includes
add_library(test_includes INTERFACE)
target_include_directories(test_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Place all test executables in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Copy JSON configs at configure time into runtime output dir and test binary dir
if (TEST_CONFIG_FILES)
    file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    file(COPY ${TEST_CONFIG_FILES} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    # CTest runs tests with working dir = ${CMAKE_CURRENT_BINARY_DIR} (build/test),
    # copy configs there so discovery / tests can open "./log4cpp.json"
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    file(COPY ${TEST_CONFIG_FILES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif ()

foreach (test_name IN LISTS TEST_TARGETS)
    # Create executable for each test
    add_executable(${test_name} ${${test_name}_SRC})

    target_include_directories(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/src/include")

    # Link against GoogleTest, main library, and common includes
    target_link_libraries(${test_name} PRIVATE GTest::gtest_main log4cpp test_includes)

    # Only load_config_tests and serialize_test need nlohmann_json
    if (test_name STREQUAL "load_config_tests" OR test_name STREQUAL "serialize_test")
        target_link_libraries(${test_name} PRIVATE nlohmann_json::nlohmann_json)
    endif ()

    # Apply AddressSanitizer if enabled
    apply_asan_to_target(${test_name})

    # Register with CTest using GoogleTest discovery
    include(GoogleTest)
    gtest_discover_tests(${test_name}
            DISCOVERY_TIMEOUT 30
            PROPERTIES TIMEOUT 30
    )
endforeach ()
