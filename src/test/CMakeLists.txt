include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

set(TEST_TARGET_NAME "MainTests")

add_test(NAME mainTest COMMAND ${TEST_TARGET_NAME} --exe $<TARGET_FILE:${TEST_TARGET_NAME}>)

add_executable(${TEST_TARGET_NAME} mainTest.cpp)
target_link_libraries(${TEST_TARGET_NAME} GTest::gtest_main log4cpp)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME} DISCOVERY_TIMEOUT 30)