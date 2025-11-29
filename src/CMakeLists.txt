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

# Check if the generator is MSVC (Visual Studio)
if (MSVC)
	# Define _CRT_SECURE_NO_WARNINGS universally for MSVC to suppress C4996 warnings
	target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

	# Conditionally define the architecture macros based on the target platform size
	# CMAKE_SIZEOF_VOID_P is 8 for 64-bit (x64/ARM64) and 4 for 32-bit (x86/ARM)
	if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		# 64-bit architecture (x64)
		target_compile_definitions(${TARGET_NAME} PRIVATE _AMD64_)
	elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		# 32-bit architecture (x86)
		target_compile_definitions(${TARGET_NAME} PRIVATE _X86_)
	endif ()
endif ()

# Includes
target_include_directories(${TARGET_NAME}
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> $<INSTALL_INTERFACE:include>
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
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
