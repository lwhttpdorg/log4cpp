set(TARGET_NAME log4cpp_demo)

add_executable(${TARGET_NAME} demo.cpp)

set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# Link to library
target_link_libraries(${TARGET_NAME} PRIVATE log4cpp)

# Optional pthread if needed (usually already linked by the library)
if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} PRIVATE pthread)
endif ()

# Copy any JSON configs from demo/ to the binary dir after build
file(GLOB DEMO_CONFIG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.json")
if (DEMO_CONFIG_FILES)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_CONFIG_FILES} $<TARGET_FILE_DIR:${TARGET_NAME}>
    )
endif ()

# Apply ASAN if enabled
apply_asan_to_target(${TARGET_NAME})
