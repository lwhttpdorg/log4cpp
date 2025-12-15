add_executable(log4cpp_demo demo.cpp)

set_target_properties(log4cpp_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# Link to library
target_link_libraries(log4cpp_demo PRIVATE log4cpp)

# Copy any JSON configs from demo/ to the binary dir after build
file(GLOB DEMO_CONFIG_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.json")
if (DEMO_CONFIG_FILES)
    add_custom_command(TARGET log4cpp_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_CONFIG_FILES} $<TARGET_FILE_DIR:log4cpp_demo>
    )
endif ()

# Apply ASAN if enabled
apply_asan_to_target(log4cpp_demo)
