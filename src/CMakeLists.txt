add_subdirectory(main)

include_directories(include)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
