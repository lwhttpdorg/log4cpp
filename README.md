# log4cpp

---

[中文版本](README_ZH.md) | English Edition

---

<!-- TOC -->
* [log4cpp](#log4cpp)
  * [1. Description](#1-description)
  * [2. Requirements](#2-requirements)
  * [3. Usage](#3-usage)
    * [3.1 Use in CMake projects](#31-use-in-cmake-projects)
    * [3.2 Configuration file](#32-configuration-file)
      * [3.2.1 Configure layout pattern](#321-configure-layout-pattern)
      * [3.2.2 Configure Appender](#322-configure-appender)
      * [3.2.3 Console Appender](#323-console-appender)
      * [3.2.4 File Appender](#324-file-appender)
      * [3.2.5 TCP Appender](#325-tcp-appender)
      * [3.2.6 UDP Appender](#326-udp-appender)
    * [3.3 Configure Layouts](#33-configure-layouts)
    * [3.4 Loading configuration file](#34-loading-configuration-file)
    * [3.5 Coding](#35-coding)
    * [3.6 Full example](#36-full-example)
    * [3.7 Contribution](#37-contribution)
      * [3.7.1 boost library](#371-boost-library)
      * [3.7.2 CMake compile options](#372-cmake-compile-options)
      * [3.7.3 Test](#373-test)
      * [3.7.4 ASAN](#374-asan)
  * [4. License](#4-license)
<!-- TOC -->

## 1. Description

log4cpp is a simple C++ log library that supports multithreading, custom output format, configuration files, console,
file, TCP, and UDP output

## 2. Requirements

1. C++ compiler that supports C++17 and above
2. CMake 3.11 and above
3. Boost >= 1.75

## 3. Usage

### 3.1 Use in CMake projects

````cmake
include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v3.0.3)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${YOUR_TARGET_NAME} log4cpp)
````

### 3.2 Configuration file

#### 3.2.1 Configure layout pattern

```json
{
	"layoutPattern": "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
}
```

Description:

* `${yy}`: A two digit representation of a year. e.g. 99 or 03
* `${yyyy}`: A full numeric representation of a year, at least 4 digits, with - for years BCE. e.g. -0055, 0787, 1999,
  2003, 10191
* `${M}`: Numeric representation of a month, without leading zeros. 1 through 12
* `${MM}`: Numeric representation of a month, with leading zeros. 01 through 12
* `${MMM}`: A short textual representation of a month, three letters. Jan through Dec
* `${d}`: Day of the month without leading zeros. 1 to 31
* `${dd}`: Day of the month, 2 digits with leading zeros. 01 to 31
* `${h}`: 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem, 0 through 12
* `${hh}`: 12-hour format of an hour with leading zeros, with Uppercase Ante meridiem and Post meridiem, 00 through 12
* `${H}`: 24-hour format of an hour without leading zeros. 0 through 23
* `${HH}`: 24-hour format of an hour with leading zeros. 00 through 23
* `${m}`: Minutes without leading zeros. 1 to 59
* `${mm}`: Minutes with leading zeros. 01 to 59
* `${s}`: Seconds without leading zeros. 1 to 59
* `${ss}`: Seconds with leading zeros. 01 to 59
* `${ms}`: Milliseconds with leading zeros. 001 to 999
* `${<n>TN}`: Thread name. e.g. `${8TN}`. `<n>` is the name length, align left, default is 16, max width is 16. If the
  name is empty, use "T+thread id" instead. e.g. "main"
* `${<n>TH}`: Thread id, e.g. `${8TH}`. `<n>` is the digit width, left padding with 0, default is 8, max width is 8.
  e.g. "T12345"
* `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${W}`: Log message, e.g. hello world!

_Warning: Some systems cannot set thread names, and only multiple threads can be distinguished by thread ID_

#### 3.2.2 Configure Appender

There are four types of appender: Console appender(`console_appender`), File appender(`file_appender`), TCP appender(
`tcp_appender`), UDP appender(`udp_appender`)

A simple configuration file example:

```json
{
	"appenders": {
		"console_appender": {
			"out_stream": "stdout"
		},
		"file_appender": {
			"file_path": "log/log4cpp.log"
		},
		"tcp_appender": {
			"local_addr": "0.0.0.0",
			"port": 9443
		},
		"udp_appender": {
			"local_addr": "0.0.0.0",
			"port": 9443
		}
	}
}
```

#### 3.2.3 Console Appender

The function of the console appender is to output logs to STDOUT or STDERR. Typical configuration is as follows:

```json
{
	"appenders": {
		"console_appender": {
			"out_stream": "stdout"
		}
	}
}
```

Description:

* `out_stream`: output stream, can be stdout or stderr

#### 3.2.4 File Appender

The function of the file appender is to output logs to a specified file. Typical configuration is as follows:

```json
{
	"appenders": {
		"file_appender": {
			"file_path": "log/log4cpp.log"
		}
	}
}
```

Description:

* `file_path`: output file name

#### 3.2.5 TCP Appender

The TCP appender will start a TCP server inside, accept TCP connections, and output logs to the connected client,
which is used to output logs to remote devices. The typical configuration is as follows:

```json
{
	"appenders": {
		"tcp_appender": {
			"local_addr": "0.0.0.0",
			"port": 9443
		}
	}
}
```

Description:

* `local_addr`: Listening address. For example, "0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: Listening port

_Note: If there are multiple TCP clients, it will be convenient for all clients to send logs one by one_

_Note: Logs are transmitted in plain text, pay attention to privacy and security issues. Encrypted transmission will not
be supported in the future. If encryption is required, it is recommended to encrypt the logs before passing them to
log4cpp_

#### 3.2.6 UDP Appender

A UDP server will be started inside the UDP log appender to export logs to the connected client, which is used to export
logs to remote devices

Unlike the TCP protocol, UDP is connectionless. Please note:

* UDP is connectionless and cannot guarantee the integrity of the log
* The client needs to actively send a "hello" message so that the server can obtain the client address and send logs
* When the client exits, it needs to send a "bye" message so that the server can clean up the client address.
  Otherwise, the client address will be retained until it is cleaned up due to log sending failure or program exit

The typical configuration is as follows:

```json
{
	"appenders": {
		"udp_appender": {
			"local_addr": "0.0.0.0",
			"port": 9443
		}
	}
}
```

Description:

* `local_addr`: listening address. For example, "0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: listening port

### 3.3 Configure Layouts

There are two types of Layouts:

* **Named layout**: configuration name `layouts`
* **Root layout**: configuration name `root_layout`

If there is no layout with a specified name when getLayout, the `root_layout` is returned

_Note: The named layout is optional, but the root layout must be present_

Named layouts are an array, and each layout configuration includes:

* `name`: layout name, used to get layouts, unique, cannot be `root`
* `log_level`: log level, only logs greater than or equal to this level will be output
* `Appenders`: appender, Must be configured in `Appenders` before it can be referenced here. Appender can be
  `console_appender`, `file_appender`, `tcp_appender`, `udp_appender`

Root layout is an object, only `log_level` and `Appenders`, no `name`, internal implementation of `name` is `root`

```json
{
	"layouts": [
		{
			"name": "console_layout",
			"log_level": "info",
			"appenders": [
				"console_appender",
				"tcp_appender",
				"udp_appender"
			]
		},
		{
			"name": "file_layout",
			"log_level": "warn",
			"appenders": [
				"file_appender"
			]
		}
	],
	"root_layout": {
		"log_level": "info",
		"appenders": [
			"console_appender",
			"file_appender",
			"tcp_appender",
			"udp_appender"
		]
	}
}
```

### 3.4 Loading configuration file

There are two ways to load configuration file:

1. If `log4cpp.json` exists in the current path, this configuration file will be automatically loaded
2. If the configuration file is not in the current path, or the file name is not `log4cpp.json`, Need to load the
   configuration file manually

```c++
log4cpp::layout_manager::load_config("/config_path/log4cpp.json");
```

### 3.5 Coding

First, you need to import the header file:

```c++
#include "log4cpp.hpp"
```

Then, get the layout instance

Get the layout by `name`. If the specified layout does not exist, return `root_layout`

```c++
std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("layout_name");
```

Write log message

After getting the layout, you can use the following method to output the log:

```c++
void trace(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void warn(const char *__restrict fmt, ...);
void error(const char *__restrict fmt, ...);
void fatal(const char *__restrict fmt, ...);
```

The above method calls the following method internally, or you can call the following method directly:

```c++
void log(log_level level, const char *fmt, ...);
```

The log level `log_level The definition of level` is as follows:

```c++
namespace log4cpp {
  enum class log_level {
    FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
  };
}
```

Description:

* `FATAL`: fatal error
* `ERROR`: error
* `WARN`: warning
* `INFO`: information
* `DEBUG`: debugging
* `TRACE`: tracing

### 3.6 Full example

```c++
#include <thread>

#ifdef __GNUC__
#include <pthread.h>
#endif

#include "log4cpp.hpp"

void set_thread_name(const char *name) {
#ifdef __GNUC__
	pthread_setname_np(pthread_self(), name);
#elif __linux__
	prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("child"));
#endif
}

void thread_routine() {
	set_thread_name("child");
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("recordLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

int main() {
	std::thread t(thread_routine);
	set_thread_name("main");
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("console_layout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
	t.join();
	return 0;
}
```

CMakeLists.txt example:

```cmake
set(TARGET_NAME demo)
add_executable(${TARGET_NAME} demo.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

file(COPY ./log4cpp.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v3.0.3)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${TARGET_NAME} log4cpp)

if (CMAKE_HOST_UNIX)
	target_link_libraries(demo pthread)
endif ()
```

Output log example:

```shell
2025-01-02 22:53:04:329 [    main] [INFO ] -- this is a info
2025-01-02 22:53:04:329 [   child] [ERROR] -- this is an error
2025-01-02 22:53:04:329 [    main] [WARN ] -- this is an warning
2025-01-02 22:53:04:329 [   child] [FATAL] -- this is a fatal
2025-01-02 22:53:04:329 [    main] [ERROR] -- this is an error
2025-01-02 22:53:04:329 [    main] [FATAL] -- this is a fatal
```

Configuration file:

[A sample configuration file is here](demo/log4cpp.json)

### 3.7 Contribution

Welcome to submit PR. There are some things to know before submitting PR:

#### 3.7.1 boost library

This project pulls the boost library from github online. You can also modify [CMakeLists.txt](src/main/CMakeLists.txt)to
use the local boost library.
Uncomment the corresponding position:

```cmake
find_package(Boost 1.75 REQUIRED COMPONENTS json)
if (Boost_FOUND)
    #message(STATUS "Boost_LIB_VERSION = ${Boost_VERSION}")
    #message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
    #message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
    #message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
endif ()
```

If CMake does not automatically find the Boost path, you can manually set the Boost path:

```cmake
if (CMAKE_HOST_WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(BOOST_ROOT "D:/OpenCode/boost/gcc")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(BOOST_ROOT "D:/OpenCode/boost/msvc")
	endif ()
else ()
	set(BOOST_ROOT "/usr/local/boost")
endif ()
```

#### 3.7.2 CMake compile options

```shell
cmake -S . -B build -DENABLE_DEMO=ON -DENABLE_TESTS=ON -DENABLE_ASAN=ON
cmake --build build --config=Debug
ctest --test-dir build
```

Option description:

* `-DENABLE_DEMO=ON`: compile demo, not compiled by default
* `-DENABLE_TEST=ON`: compile test, not enabled by default
* `-DENABLE_ASAN=ON`: enable address detection, not enabled by default

#### 3.7.3 Test

This project uses Google Test for unit testing. The test code is in the [test](src/test) directory. You are welcome to
add test cases

If your code modifies existing functions, Please make sure that the test cases cover your changes

#### 3.7.4 ASAN

If your code modifies existing functions, please make sure that the ASAN test passes. Code that does not pass the ASAN
test will not be merged

**clang_rt.asan_dynamic-x86_64.dll is missing?**

If `"ENABLE_ASAN=ON"` is set, and you are using the MSVC compiler, you may have this problem. The solution is:

Copy
`"D:\Program Files\Microsoft Visual Studio\<Visual Studio Version>\Professional\VC\Tools\MSVC\<MSVC Version>\bin\Hostx64\x64\clang_rt.asan_dynamic-x86_64.dll"`
to the `cmake-build-debug/bin/`

## 4. License

This project uses the [LGPLv3](LICENSE) license
