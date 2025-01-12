cmake_minimum_required(VERSION 3.11)

project(log4cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(INSTALL_HEAD_FILES ${INCLUDE_DIRECTORIES}/log4cpp.hpp)

install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -pipe")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -march=native -pipe")
endif ()

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	endif ()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()

if ((CMAKE_C_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -Wextra -Wshadow -Wpointer-arith -Wwrite-strings \
	-Woverloaded-virtual \
	-Wstringop-overflow \
	-Wno-pessimizing-move \
	-Wno-uninitialized -Wno-unused-parameter -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
	-Wno-unused-value -pipe")
endif ()

option(ENABLE_DEMO "Enable demo" OFF)
option(ENABLE_TESTS "Enable test" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if (MSVC AND ENABLE_ASAN)
	set(ENABLE_ASAN OFF)
	message(WARNING "MSVC does not support ASAN yet, ENABLE_ASAN is turned OFF!!!")
	#include(CheckCXXCompilerFlag)
	#check_cxx_compiler_flag("/fsanitize=address" HAS_ASAN)
	#if (NOT HAS_ASAN)
	#	set(ENABLE_ASAN OFF)
	#	message(WARNING "Your compiler does not support ASAN, ENABLE_ASAN is turned OFF!!!")
	#endif ()
endif ()

if (ENABLE_ASAN)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			message(WARNING "MinGW-w64 does not support ASAN")
			set(ENABLE_ASAN OFF)
		endif ()
	endif ()
endif ()

# AddressSanitizer
if (ENABLE_ASAN)
	if (MSVC)
		add_compile_options("/fsanitize=address")
		add_compile_options("/Zi")
		add_compile_options("/Od")

		string(LENGTH ${CMAKE_C_COMPILER} C_COMPILER_PATH_LENGTH)
		# The length of "cl.exe" is 6
		math(EXPR C_COMPILER_PATH_LENGTH "${C_COMPILER_PATH_LENGTH} - 6")
		string(SUBSTRING ${CMAKE_C_COMPILER} 0 ${C_COMPILER_PATH_LENGTH} C_COMPILER_PATH)
		file(GLOB CLANG_ASAN_DLL "${C_COMPILER_PATH}clang_rt.asan_dynamic-*.dll")
		file(COPY ${CLANG_ASAN_DLL} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
		# link clang_rt.asan_dynamic-x86_64.dll
		string(REGEX MATCH "clang_rt\\.asan_dynamic\\-[a-zA-Z0-9_]+\\.dll" CLANG_ASAN_DLL_NAME ${CLANG_ASAN_DLL})
		link_libraries(${CLANG_ASAN_DLL_NAME})
	endif ()
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		add_compile_options("-fsanitize=address")
		add_compile_options("-fsanitize=undefined")
		add_compile_options("-fno-sanitize-recover=all")
		add_compile_options("-fsanitize=float-divide-by-zero")
		add_compile_options("-fsanitize=float-cast-overflow")
		add_compile_options("-fno-sanitize=null")
		add_compile_options("-fno-sanitize=alignment")
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			add_compile_options("-fno-omit-frame-pointer")
		endif ()
		add_link_options("-fsanitize=address")
		add_link_options("-fsanitize=undefined")
	endif ()
endif ()

if (ENABLE_DEMO)
	add_subdirectory(demo)
endif ()

if (ENABLE_TESTS)
	enable_testing()
	if ((CMAKE_C_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
	endif ()
endif ()

add_subdirectory(src)

target_include_directories(log4cpp
		PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
		PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
