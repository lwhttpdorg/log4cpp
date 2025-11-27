# log4cpp

---

[中文版本](README_ZH.md) | English Edition

---

## 1. What is log4cpp?

log4cpp is a C++ logging library inspired by log4j.

Features:

- JSON configuration: change behavior without modifying code
- Output logs to STDOUT and STDERR
- Output logs to files
- Output logs to log servers (TCP/UDP)
- Singleton pattern
- Thread-safe
- Hot-reload configuration without restarting the process(Linux only)

## 2. Requirements

1. C++ compiler supporting C++17 or newer
2. CMake 3.11 or newer
3. nlohmann-json >= 3.7

_Warning: Due to a series of bugs in the MSVC compiler, this project no longer supports MSVC. Any errors on the MSVC
platform will no longer be fixed. It is recommended to use MingW64_

To install nlohmann-json on Ubuntu/Debian:

```shell
sudo apt install nlohmann-json3-dev
```

## 3. Usage

### 3.2. Configuration file

#### 3.2.1. Log output pattern

Example:

```json
{
	"log-pattern": "${NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
}
```

Placeholders:

- `${<n>NM}`: The name of logger, e.g. `${4NM}`. `<n>` is the name length, align left, default is 6, max width is 16
- `${yy}`: A two digit representation of a year. e.g. 99 or 03
- `${yyyy}`: A full numeric representation of a year, at least 4 digits, with - for years BCE. e.g. -0055, 0787, 1999,
  2003, 10191
- `${M}`: Numeric representation of a month, without leading zeros. 1 through 12
- `${MM}`: Numeric representation of a month, with leading zeros. 01 through 12
- `${MMM}`: A short textual representation of a month, three letters. Jan through Dec
- `${d}`: Day of the month without leading zeros. 1 to 31
- `${dd}`: Day of the month, 2 digits with leading zeros. 01 to 31
- `${h}`: 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem, 0 through 12
- `${hh}`: 12-hour format of an hour with leading zeros, with Uppercase Ante meridiem and Post meridiem, 00 through 12
- `${H}`: 24-hour format of an hour without leading zeros. 0 through 23
- `${HH}`: 24-hour format of an hour with leading zeros. 00 through 23
- `${m}`: Minutes without leading zeros. 1 to 59
- `${mm}`: Minutes with leading zeros. 01 to 59
- `${s}`: Seconds without leading zeros. 1 to 59
- `${ss}`: Seconds with leading zeros. 01 to 59
- `${ms}`: Milliseconds with leading zeros. 001 to 999
- `${<n>TN}`: Thread name. e.g. `${8TN}`. `<n>` is the name length, align left, default is 16, max width is 16. If the
  name is empty, use "T+thread id" instead. e.g. "main"
- `${<n>TH}`: Thread id, e.g. `${8TH}`. `<n>` is the digit width, left padding with 0, default is 8, max width is 8.
  e.g. "T12345"
- `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
- `${W}`: Log message body, e.g. hello world!

_Note: Some systems cannot set thread names; thread ID will be used instead._

#### 3.2.2. Appenders

There are four types of appenders: Console appender(`console`), File appender(`file`), Socket appender(`socket`)

Example:

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

#### 3.2.3. Console appender

Outputs logs to STDOUT or STDERR.

Example:

```json
{
	"appenders": {
		"console": {
			"out-stream": "stdout"
		}
	}
}
```

- `out_stream`: "stdout" or "stderr"

#### 3.2.4. File appender

Outputs logs to a specified file.

Example:

```json
{
	"appenders": {
		"file": {
			"file-path": "log/log4cpp.log"
		}
	}
}
```

- `file_path`: output file path

#### 3.2.5. Socket appender

Socket appender is used to send logs to remote logging servers via TCP or UDP. The protocol is specified by the
`protocol` field; if not set, TCP is used by default.

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

- `addr`: The address of remote logging server
- `port`: The port number of remote logging server
- `protocol`: "tcp" or "udp", default is "tcp"
- `prefer-stack`: "IPv4", "IPv6", or "auto", default is "auto"

_Notes: For TCP-type socket appender, if the connection to the remote logging server fails, an exponential backoff
strategy will be used for reconnection._

### 3.3. Configure loggers

There are named loggers (`loggers`) and the root logger (`root`).

- Named loggers are optional; the root logger must be present.
- If a requested logger name does not exist, `log4cpp::logger_manager::get_logger` returns the root logger.

Named logger fields:

- `name`: The logger's unique name, used for retrieval. The name `root` is reserved for the default logger.
- `level`: The logging level. Only logs with a level greater than or equal to this configured level will be output. This
  field can be omitted for non-`root` loggers (it automatically inherits the `root` logger's level).
- `appenders`: The list of appenders (outputs). Only the configured appenders will be used for output. Appenders can
  include `console`, `file`, and `socket`. This field can be omitted for non-`root` loggers (it automatically inherits
  the `root` logger's appenders).

__The default logger must be explicitly defined with the name `root`.__

Example structure:

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

### 3.4. Loading configuration

Two ways to load configuration:

1. If `log4cpp.json` exists in the current working directory, it is loaded automatically.
2. Otherwise, load manually:

```c++
const std::string config_file = "log4cpp_config_1.json";
log4cpp::logger_manager &log_mgr = log4cpp::supervisor::get_logger_manager();
log_mgr.load_config(config_file);
```

### 3.5. Hot-reload configuration

Hot-reload allows applying configuration changes without restarting (Linux only).

_Note: The config file path and name must not change; the original path/name used at startup is reloaded._

Enable hot-reload:

```c++
log4cpp::supervisor::enable_config_hot_loading(int sig = SIGHUP);
```

After modifying the configuration file, send a signal (default is `SIGHUP`) to your process:

```shell
kill -SIGHUP <PID>
```

`SIGHUP` triggers log4cpp to reload the cached file path and recreate internal objects. Existing
`std::shared_ptr<log4cpp::log::logger>` instances returned earlier remain valid until their reference count reaches
zero.

_Note: `log4cpp::logger_manager::get_logger()` may return the same shared_ptr even if the underlying proxy object
changed._

### 3.6. Usage in code

Include header:

```c++
#include <log4cpp/log4cpp.hpp>
```

Get a logger:

```c++
std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("aaa");
```

Logging methods:

```c++
void trace(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
void warn(const char *__restrict fmt, ...);
void error(const char *__restrict fmt, ...);
void fatal(const char *__restrict fmt, ...);
```

Or use the generic API:

```c++
void log(log_level level, const char *fmt, ...);
```

Log level enum:

```c++
namespace log4cpp {
    enum class log_level { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };
}
```

Notes:

- `FATAL`: fatal error
- `ERROR`: error
- `WARN`: warning
- `INFO`: information
- `DEBUG`: debugging
- `TRACE`: tracing

### 3.7. Full example

```c++
#include <thread>

#include <log4cpp/log4cpp.hpp>

void thread_routine() {
    log4cpp::set_thread_name("child");
    const auto log = log4cpp::logger_manager::get_logger("aaa");
    for (int i = 0; i < 100; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
}

int main() {
    log4cpp::supervisor::enable_config_hot_loading();
    const std::string config_file = "demo.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    std::thread child(thread_routine);
    log4cpp::set_thread_name("main");
    const auto log = log4cpp::logger_manager::get_logger("hello");

    for (int i = 0; i < 100; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
    child.join();
    return 0;
}
```

CMakeLists.txt example:

```cmake
cmake_minimum_required(VERSION 3.11)

project(log4cpp-demo)

set(TARGET_NAME demo)

add_executable(${TARGET_NAME} main.cpp)

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/lwhttpdorg/log4cpp.git GIT_TAG v3.2.1)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${TARGET_NAME} log4cpp)
```

Sample log output:

```shell
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main    ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
hello  : 2025-11-13 23:32:02:475 [main    ] [FATAL] -- this is a fatal
root   : 2025-11-13 23:32:02:475 [child   ] [INFO ] -- this is a info
hello  : 2025-11-13 23:32:02:475 [main    ] [INFO ] -- this is a info
root   : 2025-11-13 23:32:02:475 [child   ] [WARN ] -- this is an warning
hello  : 2025-11-13 23:32:02:475 [main    ] [WARN ] -- this is an warning
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main    ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
```

Reference config: [demo/demo.json](demo/demo.json)

## 4. Building

Contributions welcome. Before submitting a PR, note the following.

### 4.1. CMake build options

```shell
cmake -S . -B build -DBUILD_LOG4CPP_DEMO=ON -DBUILD_LOG4CPP_TEST=ON -DENABLE_ASAN=ON
```

```shell
cmake --build build --config=Debug -j $(nproc)
```

```shell
ctest --test-dir build --output-on-failure
```

Options:

- `-DBUILD_LOG4CPP_DEMO=ON` build demo (off by default)
- `-DBUILD_LOG4CPP_TEST=ON` build tests (off by default)
- `-DENABLE_ASAN=ON` enable AddressSanitizer (off by default)

### 4.2. Tests

This project uses Google Test; tests are under the `test` directory. Add tests as needed. Ensure changes are covered by
tests.

### 4.3. ASAN

Ensure ASAN passes for changes; PRs that fail ASAN will not be merged.

## 5. License

This project is licensed under LGPLv3 (see LICENSE).