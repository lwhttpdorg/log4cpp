add_subdirectory(main)

include_directories(include)

if (ENABLE_TEST)
	add_subdirectory(test)
endif ()
