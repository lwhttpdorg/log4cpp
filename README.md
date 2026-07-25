# log4cpp

<!-- TOC -->
- [1. What is log4cpp?](#1-what-is-log4cpp)
- [2. Requirements](#2-requirements)
- [3. Usage](#3-usage)
  - [3.1. Quick Start](#31-quick-start)
    - [3.1.1. Create the Project](#311-create-the-project)
      - [3.1.1.1. CMake](#3111-cmake)
      - [3.1.1.2. Meson](#3112-meson)
    - [3.1.2. Include Header File](#312-include-header-file)
    - [3.1.3. Load Configuration File (Optional)](#313-load-configuration-file-optional)
    - [3.1.4. Get Logger](#314-get-logger)
    - [3.1.5. Output Log](#315-output-log)
    - [3.1.6. Use in a Class](#316-use-in-a-class)
    - [3.1.7. Complete Example](#317-complete-example)
    - [3.1.8. Configuration File](#318-configuration-file)
    - [3.1.9. Build and Run](#319-build-and-run)
- [4. License](#4-license)
<!-- /TOC -->

---
[中文版](README_ZH.md) | English Version | [User Guide](docs/user_guide.md) | [Developer Guide](docs/devel.md)
---

## 1. What is log4cpp?

log4cpp is a C++ logging library inspired by log4j.

Features:

* Configurable via JSON files, no code modification required
* Supports logging to STDOUT and STDERR
* Supports logging to specified files
* Supports size/time/startup rolling for file logs, with count and history retention
* Supports logging to log server (TCP/UDP)
* Singleton pattern
* Thread-safe
* Hot configuration reload, changes take effect without restarting the process(Linux only)

## 2. Requirements

1. C++ compiler supporting C++20 or later
2. CMake 3.10 or later (for CMake builds)
3. Meson 1.1.0 or later (for Meson builds)

## 3. Usage

### 3.1. Quick Start

Create the following project:

```text
log4cpp-demo/
├── CMakeLists.txt
├── meson.build
├── subprojects/
│   └── log4cpp.wrap
├── demo.cpp
└── log4cpp.json (optional)
```

The same `demo.cpp` and optional `log4cpp.json` are used by both build systems. Keep either build definition, or both
if the project should support CMake and Meson.

#### 3.1.1. Create the Project

##### 3.1.1.1. CMake

Using `FetchContent`:

```cmake
cmake_minimum_required(VERSION 3.11)

project(log4cpp-demo LANGUAGES CXX)

add_executable(demo demo.cpp)

include(FetchContent)
FetchContent_Declare(
    log4cpp
    GIT_REPOSITORY https://github.com/lwhttpdorg/log4cpp.git
    GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(log4cpp)
target_link_libraries(demo PRIVATE log4cpp)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/log4cpp.json")
    configure_file(log4cpp.json log4cpp.json COPYONLY)
endif()
```

Or using `pkg-config` (the log4cpp deb/rpm package has already been installed):

```cmake
cmake_minimum_required(VERSION 3.11)

project(log4cpp-demo LANGUAGES CXX)

add_executable(demo demo.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LOG4CPP REQUIRED IMPORTED_TARGET log4cpp)
target_link_libraries(demo PRIVATE PkgConfig::LOG4CPP)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/log4cpp.json")
    configure_file(log4cpp.json log4cpp.json COPYONLY)
endif()
```

##### 3.1.1.2. Meson

Save the following as `meson.build`:

```meson
project(
    'log4cpp-demo',
    'cpp',
    default_options: ['cpp_std=c++20'],
    meson_version: '>=1.1.0',
)

log4cpp_dep = dependency(
    'log4cpp',
    fallback: ['log4cpp', 'log4cpp_dep'],
)

executable('demo', 'demo.cpp', dependencies: log4cpp_dep)

fs = import('fs')
if fs.exists('log4cpp.json')
    configure_file(input: 'log4cpp.json', output: 'log4cpp.json', copy: true)
endif
```

If log4cpp is not installed, create `subprojects/log4cpp.wrap` so Meson can fetch it automatically:

```ini
[wrap-git]
directory = log4cpp
url = https://github.com/lwhttpdorg/log4cpp.git
revision = v5.0.0
depth = 1
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

Get the configured logger by name:

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
void trace(std::string_view msg);
void debug(std::string_view msg);
void info(std::string_view msg);
void warn(std::string_view msg);
void error(std::string_view msg);
void fatal(std::string_view msg);

template <class... Args> void trace(std::format_string<Args...> fmt, Args &&...args);
template <class... Args> void debug(std::format_string<Args...> fmt, Args &&...args);
template <class... Args> void info(std::format_string<Args...> fmt, Args &&...args);
template <class... Args> void warn(std::format_string<Args...> fmt, Args &&...args);
template <class... Args> void error(std::format_string<Args...> fmt, Args &&...args);
template <class... Args> void fatal(std::format_string<Args...> fmt, Args &&...args);
```

Or directly:

```c++
void log(log_level level, std::string_view msg);
template <class... Args> void log(log_level level, std::format_string<Args...> fmt, Args &&...args);
```

Formatted log messages use C++20 `std::format` syntax:

```c++
logger->info("user={}, cost={}ms", user_name, cost_ms);
```

Use `at()` when a log message should include the source file and line number:

```c++
logger->at().info("user={}, cost={}ms", user_name, cost_ms);
```

The log level `log_level` is defined as follows:

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
        logger->info("func({})", name);
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
#include <memory>
#include <string>
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
        logger->info("func({})", name);
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
        log->info("this is an info");
        log->warn("this is a warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
}

int main() {
#ifndef _WIN32
    log4cpp::supervisor::enable_config_hot_loading();
#endif
    std::thread child(thread_routine);
    log4cpp::set_thread_name("main");
    const auto log = log4cpp::logger_manager::get_logger("hello");
    log->at().info("this log includes source file and line number");

    for (int i = 0; i < 10; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is an info");
        log->warn("this is a warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
    child.join();

    demo app;
    app.func("hello");

    return 0;
}
```

Example log output when using the optional configuration below:

```shell
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
hello  : 2025-11-13 23:32:02:475 [main  ] [FATAL] -- this is a fatal
root   : 2025-11-13 23:32:02:475 [child   ] [INFO ] -- this is info
hello  : 2025-11-13 23:32:02:475 [main  ] [INFO ] -- this is info
root   : 2025-11-13 23:32:02:475 [child   ] [WARN ] -- this is a warning
hello  : 2025-11-13 23:32:02:475 [main  ] [WARN ] -- this is a warning
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
```

#### 3.1.8. Configuration File

The configuration file is optional. Without one, log4cpp uses its built-in log pattern, writes to `stdout`, and uses a
`WARN`-level `root` logger.

To customize the defaults, save the following as `log4cpp.json`. log4cpp loads this filename automatically from the
current working directory; it configures console and file output, a log pattern, and named loggers:

```json
{
  "log-pattern": "${NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TN}] [${L}] -- ${msg}",
  "appenders": {
    "console": {
      "out-stream": "stdout"
    },
    "file": {
      "file-path": "log/log4cpp.log"
    }
  },
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
        "file"
      ]
    }
  ]
}
```

The repository contains a more complete [demo configuration](demo/demo.json), including a socket appender.

#### 3.1.9. Build and Run

CMake:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
./demo
```

Meson:

```shell
meson setup build-meson --buildtype=release
meson compile -C build-meson
cd build-meson
./demo
```

On Windows, run the generated `demo.exe`. When using a multi-configuration CMake generator, build with
`--config Release` and run `Release\demo.exe` from the `build` directory.

For configuration patterns, appenders, rolling policies, logger inheritance, and hot configuration reload, see the
[User Guide](docs/user_guide.md).

## 4. License

This project is licensed under [LGPLv3](LICENSE)
