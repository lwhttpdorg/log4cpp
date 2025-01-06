set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

file(GLOB CONFIG_FILES "*.json")
file(COPY ${CONFIG_FILES} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

target_include_directories(${TARGET_NAME} PUBLIC "../src/include")

target_link_libraries(${TARGET_NAME} log4cpp)

if (CMAKE_HOST_UNIX)
    target_link_libraries(demo pthread)
endif ()
