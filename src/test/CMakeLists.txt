include(FetchContent)

enable_testing()

FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

set(TEST_TARGET_NAME "MainTests")

file(COPY ./test-log4cpp-1.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ./test-log4cpp-2.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_test(NAME mainTest COMMAND ${TEST_TARGET_NAME} --exe $<TARGET_FILE:${TEST_TARGET_NAME}> WORKING_DIRECTORY "./")

add_executable(${TEST_TARGET_NAME} mainTest.cpp)
target_link_libraries(${TEST_TARGET_NAME} GTest::gtest_main log4cpp)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME} DISCOVERY_TIMEOUT 30)