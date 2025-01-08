set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELEASE_POSTFIX "")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(gtest_force_shared_crt ON)

include(FetchContent)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_CONFIG_FILES "*.json")

file(COPY ${TEST_CONFIG_FILES} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

include_directories(../)

set(LOAD_CONFIG_TEST "load_config_tests")
set(LAYOUT_PATTERN_TEST "layout_pattern_tests")
set(CONSOLE_APPENDER_TEST "console_appender_tests")
set(FILE_APPENDER_TEST "file_appender_tests")
set(TCP_APPENDER_TEST "tcp_appender_tests")
set(UDP_APPENDER_TEST "udp_appender_tests")

add_test(NAME ${LOAD_CONFIG_TEST} COMMAND ${LOAD_CONFIG_TEST} --exe $<TARGET_FILE:${LOAD_CONFIG_TEST}>)
add_test(NAME ${LAYOUT_PATTERN_TEST} COMMAND ${LAYOUT_PATTERN_TEST} --exe $<TARGET_FILE:${LAYOUT_PATTERN_TEST}>)
add_test(NAME ${CONSOLE_APPENDER_TEST} COMMAND ${CONSOLE_APPENDER_TEST} --exe $<TARGET_FILE:${CONSOLE_APPENDER_TEST}>)
add_test(NAME ${FILE_APPENDER_TEST} COMMAND ${FILE_APPENDER_TEST} --exe $<TARGET_FILE:${FILE_APPENDER_TEST}>)
add_test(NAME ${TCP_APPENDER_TEST} COMMAND ${TCP_APPENDER_TEST} --exe $<TARGET_FILE:${TCP_APPENDER_TEST}>)
add_test(NAME ${UDP_APPENDER_TEST} COMMAND ${UDP_APPENDER_TEST} --exe $<TARGET_FILE:${UDP_APPENDER_TEST}>)

FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(${LOAD_CONFIG_TEST} load_config_test.cpp)
add_executable(${LAYOUT_PATTERN_TEST} layout_pattern_test.cpp)
add_executable(${CONSOLE_APPENDER_TEST} console_appender_test.cpp)
add_executable(${FILE_APPENDER_TEST} file_appender_test.cpp)
add_executable(${TCP_APPENDER_TEST} tcp_appender_test.cpp)
add_executable(${UDP_APPENDER_TEST} udp_appender_test.cpp)

target_link_libraries(${LOAD_CONFIG_TEST} GTest::gtest_main nlohmann_json::nlohmann_json log4cpp)
target_link_libraries(${LAYOUT_PATTERN_TEST} GTest::gtest_main log4cpp)
target_link_libraries(${CONSOLE_APPENDER_TEST} GTest::gtest_main log4cpp)
target_link_libraries(${FILE_APPENDER_TEST} GTest::gtest_main log4cpp)
target_link_libraries(${TCP_APPENDER_TEST} GTest::gtest_main log4cpp)
target_link_libraries(${UDP_APPENDER_TEST} GTest::gtest_main log4cpp)

include(GoogleTest)
gtest_discover_tests(${LOAD_CONFIG_TEST} DISCOVERY_TIMEOUT 30)
gtest_discover_tests(${LAYOUT_PATTERN_TEST} DISCOVERY_TIMEOUT 30)
gtest_discover_tests(${CONSOLE_APPENDER_TEST} DISCOVERY_TIMEOUT 30)
gtest_discover_tests(${FILE_APPENDER_TEST} DISCOVERY_TIMEOUT 30)
gtest_discover_tests(${TCP_APPENDER_TEST} DISCOVERY_TIMEOUT 30)
gtest_discover_tests(${UDP_APPENDER_TEST} DISCOVERY_TIMEOUT 30)
