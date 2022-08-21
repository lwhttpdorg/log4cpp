add_subdirectory(main)

if (ENABLE_DEMO)
    add_subdirectory(demo)
endif ()
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()