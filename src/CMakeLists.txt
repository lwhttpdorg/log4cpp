add_subdirectory(main)

include_directories(include)

if (ENABLE_TESTS)
    add_subdirectory(test)
endif ()
