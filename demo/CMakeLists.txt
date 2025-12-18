add_executable(log4cpp_demo demo.cpp)

set_target_properties(log4cpp_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# Link to library
target_link_libraries(log4cpp_demo PRIVATE log4cpp)

file(GLOB DEMO_CONFIG_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.json")
if (DEMO_CONFIG_FILES)
    # Copy the configuration files to build/bin so that the program can run
    file(COPY ${DEMO_CONFIG_FILES} DESTINATION "${PROJECT_BINARY_DIR}/bin")
endif ()

# Apply ASAN if enabled
apply_asan_to_target(log4cpp_demo)
