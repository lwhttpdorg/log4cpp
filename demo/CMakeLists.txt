set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.cpp)

file(COPY ./log4cpp.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})

target_include_directories(${TARGET_NAME} PUBLIC "../src/include")

target_link_libraries(${TARGET_NAME} log4cpp)

if (CMAKE_HOST_UNIX)
    target_link_libraries(demo pthread)
endif ()
