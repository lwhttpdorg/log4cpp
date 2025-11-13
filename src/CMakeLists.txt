set(TARGET_NAME log4cpp)

# Sources
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp")

# Library type
if (CMAKE_HOST_WIN32)
	add_library(${TARGET_NAME} STATIC ${SRC_FILES})
else ()
	add_library(${TARGET_NAME} SHARED ${SRC_FILES})
endif ()

# C++ standard scoped to target
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

# Includes
target_include_directories(${TARGET_NAME}
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform specifics
if (CMAKE_HOST_WIN32)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_definitions(${TARGET_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
	endif ()
	target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
elseif (CMAKE_HOST_UNIX)
	target_link_libraries(${TARGET_NAME} PRIVATE pthread)
endif ()

# GNU warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(${TARGET_NAME} PRIVATE
			-Wall -Wold-style-cast -Wextra -Wshadow -Wpointer-arith -Wwrite-strings
			-Woverloaded-virtual -Wstringop-overflow -Wno-pessimizing-move
			-Wno-uninitialized -Wno-unused-parameter -Wno-unused-function
			-Wno-unused-variable -Wno-unused-value -pipe
	)
endif ()

target_link_libraries(${TARGET_NAME} PUBLIC nlohmann_json::nlohmann_json)

# Output dirs
set_target_properties(${TARGET_NAME} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
		LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
		RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# Apply ASAN
apply_asan_to_target(${TARGET_NAME})

# Install rules
install(TARGETS ${TARGET_NAME}
		EXPORT log4cppTargets
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/log4cpp
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT log4cppTargets
		FILE log4cppTargets.cmake
		NAMESPACE log4cpp::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/log4cpp
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/log4cppConfigVersion.cmake"
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY SameMajorVersion
)

#configure_package_config_file(
#		"${CMAKE_SOURCE_DIR}/cmake/log4cppConfig.cmake.in"
#		"${CMAKE_CURRENT_BINARY_DIR}/log4cppConfig.cmake"
#		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/log4cpp
#)

#install(FILES
#		"${CMAKE_CURRENT_BINARY_DIR}/log4cppConfig.cmake"
#		"${CMAKE_CURRENT_BINARY_DIR}/log4cppConfigVersion.cmake"
#		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/log4cpp
#)
