# log4cpp

---
[中文版本](README_ZH.md) | English Version
---

<!-- TOC -->
* [log4cpp](#log4cpp)
  * [中文版本 | English Version](#中文版本--english-version)
  * [1. What is log4cpp?](#1-what-is-log4cpp)
  * [2. Requirements](#2-requirements)
  * [3. Usage](#3-usage)
    * [3.1. Quick Start](#31-quick-start)
      * [3.1.1. Create a CMake Project](#311-create-a-cmake-project)
      * [3.1.2. Include Header File](#312-include-header-file)
      * [3.1.3. Load Configuration File (Optional)](#313-load-configuration-file-optional)
      * [3.1.4. Get Logger](#314-get-logger)
      * [3.1.5. Output Log](#315-output-log)
      * [3.1.6. Use in a Class](#316-use-in-a-class)
      * [3.1.7. Complete Example](#317-complete-example)
    * [3.2. Advanced Usage](#32-advanced-usage)
      * [3.2.1. Configuration File](#321-configuration-file)
        * [3.2.1.1. Log pattern](#3211-log-pattern)
        * [3.2.1.2. Appender](#3212-appender)
          * [3.2.1.2.1. Console Appender](#32121-console-appender)
          * [3.2.1.2.2. File Appender](#32122-file-appender)
      * [3.2.2. Socket appender](#322-socket-appender)
      * [3.2.3. Logger](#323-logger)
    * [3.3. Hot Configuration Reload](#33-hot-configuration-reload)
  * [4. Building](#4-building)
    * [4.1. Configuration](#41-configuration)
      * [4.1.1. Windows](#411-windows)
      * [4.1.2. Linux](#412-linux)
    * [4.2. Build](#42-build)
    * [4.3. Testing](#43-testing)
    * [4.4. Build RPM/DEB](#44-build-rpmdeb)
    * [4.5. ASAN](#45-asan)
  * [5. License](#5-license)
<!-- TOC -->

## 1. What is log4cpp?

log4cpp is a C++ logging library inspired by log4j

Features:

* Configurable via JSON files, no code modification required
* Supports logging to STDOUT and STDERR
* Supports logging to specified files
* Supports logging to log server (TCP/UDP)
* Singleton pattern
* Thread-safe
* Hot configuration reload, changes take effect without restarting the process(Linux only)

## 2. Requirements

1. C++ compiler supporting C++17 or later
2. CMake 3.11 or later
3. nlohmann-json >= 3.0

## 3. Usage

### 3.1. Quick Start

To install nlohmann-json on Ubuntu/Debian:

```shell
sudo apt install nlohmann-json3-dev
```

#### 3.1.1. Create a CMake Project

CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.11)

project(log4cpp-demo)

add_executable(demo main.cpp)

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/lwhttpdorg/log4cpp.git GIT_TAG v4.0.4)
FetchContent_MakeAvailable(log4cpp)
target_link_libraries(demo log4cpp)
```

#### 3.1.2. Include Header File

Header file:

```c++
#include <log4cpp/log4cpp.hpp>
```

#### 3.1.3. Load Configuration File (Optional)

Configuration can be loaded in two ways:

* If `log4cpp.json` exists in the current path, it will be loaded automatically
* If the configuration file is not in the current path, or has a different name, you need to load it manually

_Notes: If `log4cpp.json` does not exist and is not loaded manually, the built-in default configuration will be used._

```c++
const std::string config_file = "demo.json";
auto &log_mgr = log4cpp::supervisor::get_logger_manager();
log_mgr.load_config(config_file);
```

#### 3.1.4. Get Logger

Get the configured logger by name

```c++
std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger(const std::string &name = "root");
```

You can specify a unique string, which can be output to the log (the length of the output can be specified via
`${<n>NM}` in "log-pattern")

```shell
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
```

#### 3.1.5. Output Log

After getting the logger, you can use the following methods to output the log:

```shell
void trace(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
void warn(const char *__restrict fmt, ...);
void error(const char *__restrict fmt, ...);
void fatal(const char *__restrict fmt, ...);
```

Or directly:

```c++
void log(log_level level, const char *fmt, ...);
```

The log level `log_level` level is defined as follows:

```c++
namespace log4cpp {
  enum class log_level { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };
}
```

Description:

* `FATAL`: Fatal error
* `ERROR`: Error
* `WARN`: Warning
* `INFO`: Information
* `DEBUG`: Debugging
* `TRACE`: Tracing

#### 3.1.6. Use in a Class

The logger object can also be used as a class member variable (or static member variable). Since it is a
`std::shared_ptr`, all instances of the class will use the same logger

```c++
class demo {
public:
    demo() {
        logger = log4cpp::logger_manager::get_logger("demo");
        logger->info("constructor");
    }

    ~demo() {
        logger->info("destructor");
    }

    void func(const std::string &name) const {
        logger->info("func(%s)", name.c_str());
    }

private:
    std::shared_ptr<log4cpp::logger> logger;
};
```

You will get the following log:

```shell
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- constructor
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- func(hello)
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- destructor
```

#### 3.1.7. Complete Example

```c++
#include <thread>

#include <log4cpp/log4cpp.hpp>

class demo {
public:
    demo() {
        logger = log4cpp::logger_manager::get_logger("demo");
        logger->info("constructor");
    }

    ~demo() {
        logger->info("destructor");
    }

    void func(const std::string &name) const {
        logger->info("func(%s)", name.c_str());
    }

private:
    std::shared_ptr<log4cpp::logger> logger;
};

void thread_routine() {
    log4cpp::set_thread_name("child");
    const auto log = log4cpp::logger_manager::get_logger("aaa");
    for (int i = 0; i < 10; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
}

int main() {
#ifndef _WIN32
    log4cpp::supervisor::enable_config_hot_loading();
#endif
    const std::string config_file = "demo.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    std::thread child(thread_routine);
    log4cpp::set_thread_name("main");
    const auto log = log4cpp::logger_manager::get_logger("hello");

    for (int i = 0; i < 10; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
    child.join();

    demo app;
    app.func("hello");

    return 0;
}
```

Example Log Output:

```shell
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
hello  : 2025-11-13 23:32:02:475 [main  ] [FATAL] -- this is a fatal
root   : 2025-11-13 23:32:02:475 [child   ] [INFO ] -- this is a info
hello  : 2025-11-13 23:32:02:475 [main  ] [INFO ] -- this is a info
root   : 2025-11-13 23:32:02:475 [child   ] [WARN ] -- this is an warning
hello  : 2025-11-13 23:32:02:475 [main  ] [WARN ] -- this is an warning
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
```

Configuration File Example:

Reference configuration file [demo/demo.json](demo/demo.json)

### 3.2. Advanced Usage

#### 3.2.1. Configuration File

##### 3.2.1.1. Log pattern

```json
{
	"log-pattern": "${NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${msg}"
}
```

Placeholders:

* `${<n>NM}`: Logger name, e.g. `${8NM}`. `<n>` is the logger name length, left-aligned, default is 6, max is 64
* `${yy}`: Year represented by 2 digits. e.g. 99 or 03
* `${yyyy}`: Full year, at least 4 digits, using '-' for BC e.g. -0055, 0787, 1999, 2003, 10191
* `${M}`: Month in number, without leading zero. From 1 to 12
* `${MM}`: Month in number, two digits with leading zero. From 01 to 12
* `${MMM}`: Abbreviated month name, 3 letters. From Jan to Dec
* `${d}`: Day of the month, without leading zero. From 1 to 31
* `${dd}`: Day of the month, two digits with leading zero. From 01 to 31
* `${h}`: Hour in 12-hour clock without leading zero. AM and PM for morning and afternoon. From 0 to 12
* `${hh}`: Hour in 12-hour clock with leading zero. AM and PM for morning and afternoon. From 00 to 12
* `${H}`: Hour in 24-hour clock without leading zero. From 0 to 23
* `${HH}`: Hour in 24-hour clock with leading zero. From 00 to 23
* `${m}`: Minute without leading zero. From 1 to 59
* `${mm}`: Minute with leading zero. From 01 to 59
* `${s}`: Second without leading zero. From 1 to 59
* `${ss}`: Second with leading zero. From 01 to 59
* `${ms}`: Millisecond with leading zero. From 001 to 999
* `${<n>TN}`: is the thread name length, left-aligned, default is 16, max is 16. If the thread name is empty, "T+$
  {Thread ID}" is used instead, e.g., "main", "T12345"
* `${<n>TH}`: Thread id, e.g. `${8TH}`. `<n>` is the number of digits for the Thread ID, left-padded with 0, default is
  8, max is 8. e.g. "T12345"
* `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${msg}`: Log message body, e.g. hello world!

_Note: Some systems cannot set thread names, and multiple threads can only be distinguished by Thread ID_

Note: The default log-pattern is `"${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${msg}"`

##### 3.2.1.2. Appender

There are four types of appenders: Console Appender(`console`), File Appender(`file`), Socket Appender(`socket`, default
is TCP)

A simple configuration file example:

```json
{
	"appenders": {
		"console": {
			"out-stream": "stdout"
		},
		"file": {
			"file-path": "log/log4cpp.log"
		},
		"socket": {
			"host": "10.0.0.1",
			"port": 9443,
			"protocol": "tcp",
			"prefer-stack": "auto"
		}
	}
}
```

###### 3.2.1.2.1. Console Appender

The Console Appender's function is to output logs to `STDOUT` or `STDERR`. Typical configuration is as follows:

```json
{
	"appenders": {
		"console": {
			"out-stream": "stdout"
		}
	}
}
```

Description:

* `out-stream`: Output stream, can be "stdout" or "stderr"

###### 3.2.1.2.2. File Appender

The File Appender's function is to output logs to a specified file. Typical configuration is as follows:

```json
{
	"appenders": {
		"file": {
			"file-path": "log/log4cpp.log"
		}
	}
}
```

Description:

* `file-path`: output file name

#### 3.2.2. Socket appender

The Socket Appender supports both TCP and UDP protocols, distinguished by the `protocol` field. If `protocol` is not
configured, it defaults to `TCP`

```json
{
	"appenders": {
		"socket": {
			"host": "10.0.0.1",
			"port": 9443,
			"protocol": "tcp",
			"prefer-stack": "auto"
		}
	}
}
```

Description:

* `host`: Remote log server hostname
* `port`: Remote log server port
* `protocol`: Protocol, can be "tcp" or "udp", default is "tcp"
* `prefer-stack`: Preferred address stack, can be "IPv4", "IPv6", or "auto", default is "AUTO"

_Notes: For TCP-type socket appender, if the connection to the remote logging server fails, it will attempt to reconnect
with exponential backoff until the connection succeeds_

#### 3.2.3. Logger

`loggers` is an array. Each logger configuration includes:

* `name`: Logger name, used to retrieve the logger, must be unique. `root` is the default logger
* `level`: Log level. Only logs greater than or equal to this level will be output. Can be omitted for non-`root`
  loggers (automatically inherits from `root`)
* `appenders`: Appenders. Only configured appenders will output logs. Appenders can be `console`, `file`, `socket`. Can
  be omitted for non-`root` loggers (automatically inherits from `root`)

__The default logger must be defined with name `root`__

```json
{
	"loggers": [
		{
			"name": "root",
			"level": "INFO",
			"appenders": [
				"console",
				"file"
			]
		},
		{
			"name": "hello",
			"level": "INFO",
			"appenders": [
				"console",
				"socket"
			]
		},
		{
			"name": "aaa",
			"level": "WARN",
			"appenders": [
				"file"
			]
		}
	]
}
```

### 3.3. Hot Configuration Reload

Configuration hot reloading allows changes to the configuration file to take effect without restarting the process (
Linux system only)

_Note: The configuration file path and name cannot be changed; the path and name used at startup will be reloaded._

First, you need to enable configuration hot loading:

```c++
log4cpp::supervisor::enable_config_hot_loading(int sig = SIGHUP);
```

After modifying the configuration file, send a signal to your process (default is SIGHUP):

```shell
kill -SIGHUP <PID>
```

The `SIGHUP` signal will trigger log4cpp to reload the configuration file using the cached path and filename, and
recreate internal objects. The `std::shared_ptr<log4cpp::logger>` previously obtained via
`log4cpp::logger_manager::get_logger()` will not become invalid and can continue to be used

_Note: The `std::shared_ptr` returned by `log4cpp::logger_manager::get_logger()` may not change, even if its internal
proxy object has changed

## 4. Building

### 4.1. Configuration

#### 4.1.1. Windows

MingW64:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DBUILD_LOG4CPP_TEST=ON -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/OpenCode/nlohmann_json"
```

MSVC:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DBUILD_LOG4CPP_TEST=ON -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="D:/OpenCode/nlohmann_json"
```

#### 4.1.2. Linux

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DBUILD_LOG4CPP_TEST=ON -DENABLE_ASAN=ON
```

Options:

* `DBUILD_LOG4CPP_DEMO=ON`: Build demo, default `OFF` (not build)
* `DBUILD_LOG4CPP_TEST=ON`: Build test programs , default `OFF` (not build)
* `DENABLE_ASAN=ON`: Enable AddressSanitizer, default `OFF` (not enabled)

### 4.2. Build

```shell
cmake --build build -j $(nproc)
```

### 4.3. Testing

This project uses [Google Test](https://github.com/google/googletest) for unit testing

```shell
ctest -C Debug --test-dir build --output-on-failure
```

Or enable more verbose output from tests:

```shell
ctest -C Debug --test-dir build --verbose
```

### 4.4. Build RPM/DEB

Build DEB:

```shell
fakeroot debian/rules clean
DEB_BUILD_OPTIONS="noddebs" dpkg-buildpackage -us -uc -b -j$(nproc)
```
Build RPM:

```shell
rpmdev-setuptree
tar -czf ~/rpmbuild/SOURCES/liblog4cpp-4.0.4.tar.gz log4cpp/
cp log4cpp/liblog4cpp.spec rpmbuild/SPECS/
rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec
```

### 4.5. ASAN

If your code modifies existing functionality, please ensure that ASAN detection passes. Code that has not passed ASAN
detection will not be merged

## 5. License

This project is licensed under [LGPLv3](LICENSE)