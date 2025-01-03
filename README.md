# log4cpp

---

[中文版本](README_ZH.md) | English

---

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
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v2.0.1)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${YOUR_TARGET_NAME} log4cpp)
````

### 3.2 Configuration file

#### 3.2.1 Configure output format

```json
{
  "pattern": "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
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
* `${h}`: 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. e.g. AM
  01 or PM 11
* `${hh}`: 24-hour format of an hour with leading zeros. 00 through 23
* `${m}`: Minutes without leading zeros. 1 to 59
* `${mm}`: Minutes with leading zeros. 01 to 59
* `${s}`: Seconds without leading zeros. 1 to 59
* `${ss}`: Seconds with leading zeros. 01 to 59
* `${ms}`: Milliseconds with leading zeros. 001 to 999
* `${TH}`: The name of the thread, if the name is empty, use "T+thread id" instead, e.g. "main", "T12345"
* `${\d+TH}`: The regular expression to match the thread id pattern, e.g. ${8TH}. `\d+` is the digit width, default is 8, max width is
  16
* `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${W}`: Log message, e.g. hello world!

_Warning: Some systems cannot set thread names, and only multiple threads can be distinguished by thread ID_

#### 3.2.2 Configure log output

There are four types of configured output: Console log output(consoleOutPut), File log output(fileOutPut), TCP log
output(
tcpOutPut), UDP log output(udpOutPut)

A simple configuration file example:

```json
{
  "logOutPut": {
	"consoleOutPut": {
	  "outStream": "stdout"
	},
	"fileOutPut": {
	  "filePath": "log/log4cpp.log",
	  "append": false
	},
	"tcpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	},
	"udpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	}
  }
}
```

#### 3.2.3 Console log output

The function of the console output is to output logs to STDOUT or STDERR. Typical configuration is as follows:

```json
{
  "logOutPut": {
	"consoleOutPut": {
	  "outStream": "stdout"
	}
  }
}
```

Description:

* `outStream`: output stream, can be stdout or stderr

#### 3.2.4 File log output

The function of the file output is to output logs to a specified file. Typical configuration is as follows:

```json
{
  "logOutPut": {
	"fileOutPut": {
	  "filePath": "log/log4cpp.log",
	  "append": true
	}
  }
}
```

Description:

* `filePath`: output file name
* `append`: append or overwrite, Default append (true)

#### 3.2.5 TCP log output

The TCP log output will start a TCP server inside, accept TCP connections, and output logs to the connected client,
which
is used to output logs to remote devices. The typical configuration is as follows:

```json
{
  "logOutPut": {
	"tcpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	}
  }
}
```

Description:

* `localAddr`: Listening address. For example, "0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: Listening port

_Note: If there are multiple TCP clients, it will be convenient for all clients to send logs one by one_

_Note: Logs are transmitted in plain text, pay attention to privacy and security issues. Encrypted transmission will not
be supported in the future. If encryption is required, it is recommended to encrypt the logs before passing them to
log4cpp_

#### 3.2.6 UDP log output

A UDP server will be started inside the UDP log output to export logs to the connected client, which is used to export
logs to remote devices

Unlike the TCP log output, UDP is connectionless. Please note:

* UDP is connectionless and cannot guarantee the integrity of the log
* The client needs to actively send a "hello" message so that the server can obtain the client address and send logs
* When the client exits, it needs to send a "bye" message so that the server can clean up the client address.
  Otherwise, the client address will be retained until it is cleaned up due to log sending failure or program exit

The typical configuration is as follows:

```json
{
  "logOutPut": {
	"udpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	}
  }
}
```

Description:

* `localAddr`: listening address. For example, "0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: listening port

### 3.3 Configure logger

Loggers are divided into named loggers (configuration name `loggers`) and default loggers (configuration
name `rootLogger`). If there is no logger with a specified name when getLogger, the default logger is returned

_Note: named loggers can be absent, but default loggers must be present_

Named loggers are an array, and each logger configuration includes:

* `name`: logger name, used to get loggers, cannot be repeated, cannot be `root`
* `logLevel`: log level, only logs greater than or equal to this level will be output
* `logOutPuts`: output device, only configured output devices will be output. Output devices can
  be `consoleOutPut`, `fileOutPut`, `tcpOutPut`, `udpOutPut`

Default logger is an object, only `logLevel` and `logOutPuts`, no `name`, internal implementation of `name` is `root`

```json
{
  "loggers": [
	{
	  "name": "consoleLogger",
	  "logLevel": "info",
	  "logOutPuts": [
		"consoleOutPut"
	  ]
	},
	{
	  "name": "recordLogger",
	  "logLevel": "error",
	  "logOutPuts": [
		"fileOutPut",
		"tcpOutPut",
		"udpOutPut"
	  ]
	}
  ],
  "rootLogger": {
	"logLevel": "info",
	"logOutPuts": [
	  "fileOutPut",
	  "tcpOutPut",
	  "udpOutPut"
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
log4cpp::logger_manager::load_config("/config_path/log4cpp.json");
```

### 3.5 Coding

First, you need to import the header file:

```c++
#include "log4cpp.hpp"

```

Then get the logger instance. Get the logger through `name`. If the specified logger does not exist, then return the
default `rootLogger`

```c++
std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
```

After getting the logger, you can use the following method to output the log:

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

### 3.6 Complete example

```c++
#include <thread>

#include "log4cpp.hpp"

void set_thread_name(const char *name) {
#ifdef _PTHREAD_H
	pthread_setname_np(pthread_self(), name);
#elif __linux__
	prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("child"));
#endif
}

void thread_routine() {
	set_thread_name("child");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("this is a trace");
	logger->info("this is a info");
	logger->debug("this is a debug");
	logger->warn("this is an warning");
	logger->error("this is an error");
	logger->fatal("this is a fatal");
}

int main() {
	std::thread t(thread_routine);
	set_thread_name("main");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("this is a trace");
	logger->info("this is a info");
	logger->debug("this is a debug");
	logger->warn("this is an warning");
	logger->error("this is an error");
	logger->fatal("this is a fatal");
	t.join();
	return 0;
}
```

CMakeLists.txt example:

```cmake
set(TARGET_NAME demo)
add_executable(${TARGET_NAME} demo.cpp)

file(COPY ./log4cpp.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v2.0.1)

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

Configuration file example:

[Reference configuration file example](demo/log4cpp.json)

```json
{
  "pattern": "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}",
  "logOutPut": {
	"consoleOutPut": {
	  "outStream": "stdout"
	},
	"fileOutPut": {
	  "filePath": "log/log4cpp.log",
	  "append": true
	},
	"tcpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	},
	"udpOutPut": {
	  "localAddr": "0.0.0.0",
	  "port": 9443
	}
  },
  "loggers": [
	{
	  "name": "consoleLogger",
	  "logLevel": "info",
	  "logOutPuts": [
		"consoleOutPut",
		"tcpOutPut",
		"udpOutPut"
	  ]
	},
	{
	  "name": "recordLogger",
	  "logLevel": "error",
	  "logOutPuts": [
		"consoleOutPut",
		"fileOutPut",
		"tcpOutPut",
		"udpOutPut"
	  ]
	}
  ],
  "rootLogger": {
	"logLevel": "info",
	"logOutPuts": [
	  "fileOutPut",
	  "tcpOutPut",
	  "udpOutPut"
	]
  }
}
```

### 3.7 Contribution

Welcome to submit PR. There are some things to know before submitting PR:

#### 3.7.1 boost library

This project pulls the boost library from github online. You can also modify [CMakeLists.txt](src/main/CMakeLists.txt)to
use the local boost library.
Uncomment the corresponding position:

```cmake
#find_package(Boost 1.75 REQUIRED COMPONENTS json)
#if (Boost_FOUND)
#    #message(STATUS "Boost_LIB_VERSION = ${Boost_VERSION}")
#    #message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
#    #message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
#    #message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
#    include_directories(${Boost_INCLUDE_DIRS})
#    link_directories(${Boost_LIBRARY_DIRS})
#    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
#endif ()
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
$ cmake -S . -B build -DENABLE_DEMO=ON
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

## 4. License

This project uses the [GPLv3] (LICENSE) license
