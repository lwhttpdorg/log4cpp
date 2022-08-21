add_subdirectory(main)
add_subdirectory(demo)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()