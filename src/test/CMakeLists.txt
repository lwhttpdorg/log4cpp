include(FetchContent)

enable_testing()

set(gtest_force_shared_crt ON)

FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

set(TEST_TARGET_NAME "MainTests")

file(GLOB TEST_CONFIG_FILES "*.json")

file(COPY ${TEST_CONFIG_FILES} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_test(NAME mainTest COMMAND ${TEST_TARGET_NAME} --exe $<TARGET_FILE:${TEST_TARGET_NAME}> WORKING_DIRECTORY "./")

include_directories(../)

add_executable(${TEST_TARGET_NAME} mainTest.cpp tcp_appender_test.cpp udp_appender_test.cpp)
target_link_libraries(${TEST_TARGET_NAME} GTest::gtest_main log4cpp)

if (ENABLE_ASAN)
    target_link_options(${TEST_TARGET_NAME} PRIVATE "-fsanitize=address -fsanitize=undefined")
endif ()

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME} DISCOVERY_TIMEOUT 30)