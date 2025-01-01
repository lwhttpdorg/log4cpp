# log4cpp

## 1. 简述

Logger for C++是一个为C++开发的日志记录工具, 可以将log输出到控制台或指定的文件

## 2. 特性

- 支持Linux/Windows
- 线程安全

## 3. 要求

1. 支持C++17及以上的C++编译器
2. CMake 3.11及以上版本
3. Boost >= 1.75

## 3. 使用

### 3.1 编译和安装

如果CMake没有自动找到Boost路径, 可参考修改[CMakeLists.txt](CMakeLists.txt)

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

```shell
$ cmake -S . -B build -DENABLE_DEMO=ON
$ cd build
$ make
```

### 3.2 使用

#### a. 头文件

```c++
#include "log4cpp.hpp"
```

#### b. CMake

CMakeLists.txt示例:

```cmake
add_executable(${TARGET_NAME} main.cpp)

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v2.0.1)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${TARGET_NAME} log4cpp)
```

#### c. API

1. 加载配置文件

如果当前路径下存在`log4cpp.json`, 会自动加载此配置文件. 如果配置文件不在当前路径或文件名不是`log4cpp.json`, 需要手动加载配置文件

```c++
log4cpp::logger_manager::load_config("./log4cpp.json");
```

2. 获取logger实例

通过`name`获取配置的`"name": "consoleLogger"`logger, 如果不存在指定的logger, 则返回默认的`rootLogger`

```c++
std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
```

3. 输出log

```c++
void log(log_level level, const char *fmt, ...);
```

下面几个API是上面的特化

```c++
void trace(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void warn(const char *__restrict fmt, ...);
void error(const char *__restrict fmt, ...);
void fatal(const char *__restrict fmt, ...);
```

#### d. demo

```c++
#include <pthread.h>

#include "log4cpp.hpp"

void *child_thread_routine(void *) {
	pthread_setname_np(pthread_self(), "child");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("Child: this is a trace 0x%x", pthread_self());
	logger->info("Child: this is a info 0x%x", pthread_self());
	logger->debug("Child: this is a debug 0x%x", pthread_self());
	logger->error("Child: this is an error 0x%x", pthread_self());
	logger->fatal("Child: this is a fatal 0x%x", pthread_self());
	return nullptr;
}

int main() {
	pthread_t child_tid;
	pthread_create(&child_tid, nullptr, child_thread_routine, nullptr);
	pthread_setname_np(pthread_self(), "main");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("Main: this is a trace 0x%x", pthread_self());
	logger->info("Main: this is a info 0x%x", pthread_self());
	logger->debug("Main: this is a debug 0x%x", pthread_self());
	logger->error("Main: this is an error 0x%x", pthread_self());
	logger->fatal("Main: this is a fatal 0x%x", pthread_self());

	pthread_join(child_tid, nullptr);

	return 0;
}
```

### 3.3 附加说明

#### a. 输出格式

```text
${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss} [${8TH}] [${L}] -- ${W}
```

解释:

* `${yy}`: A two digit representation of a year. Examples: 99 or 03
* `${yyyy}`: A full numeric representation of a year, at least 4 digits, with - for years BCE. Examples: -0055, 0787,
  1999, 2003, 10191
* `${M}`: Numeric representation of a month, without leading zeros. 1 through 12
* `${MM}`: Numeric representation of a month, with leading zeros. 01 through 12
* `${MMM}`: A short textual representation of a month, three letters. Jan through Dec
* `${d}`: Day of the month without leading zeros. 1 to 31
* `${dd}`: Day of the month, 2 digits with leading zeros. 01 to 31
* `${h}`: 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. Examples: AM
  01 or PM 11
* `${hh}`: 24-hour format of an hour with leading zeros. 00 through 23
* `${m}`: Minutes without leading zeros. 1 to 59
* `${mm}`: Minutes with leading zeros. 01 to 59
* `${s}`: Seconds without leading zeros. 1 to 59
* `${ss}`: Seconds with leading zeros. 01 to 59
* `${TH}`: The name of the thread, if the name is empty, use thread id instead, e.g. T12345
* `${\d+TH}`: The regular expression to match the thread id pattern, e.g. ${8TH}. max width is 16. if the name is empty,
  use thread id instead, e.g. T12345
* `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${W}`: Log content, Examples: hello world!

其中:

1. 秒精确到小数点后三位(毫秒)
2. `${thread id}`为线程ID, 某些系统无法设置线程名, 只能通过线程ID区分多线程
3. log级别的定义如下:

```c++
namespace log4cpp {
	enum class log_level {
		FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
	};
}
```

示例:

```shell
2024-03-09 18:33:24 [    main] [INFO ] - Main: this is a info 0x9b4b0b80
2024-03-09 18:33:24 [   child] [ERROR] - Child: this is an error 0x9adff640
2024-03-09 18:33:24 [    main] [WARN ] - Main: this is an warning 0x9b4b0b80
2024-03-09 18:33:24 [    main] [ERROR] - Main: this is an error 0x9b4b0b80
2024-03-09 18:33:24 [   child] [FATAL] - Child: this is a fatal 0x9adff640
2024-03-09 18:33:24 [    main] [FATAL] - Main: this is a fatal 0x9b4b0b80
```

## 4. 配置文件示例

[参考配置文件示例](demo/log4cpp.json)

```json
{
  // 输出格式
  "pattern": "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss} [${8TH}] [${L}] -- ${W}",
  // 输出器
  "logOutPut": {
    // 控制台输出器
    "consoleOutPut": {
      // 输出流，可以是stdout或stderr
      "outStream": "stdout"
    },
    // 文件输出器
    "fileOutPut": {
      // 输出文件
      "filePath": "log/log4cpp.log",
      // 追加还是覆盖, 默认覆盖
      "append": false
    },
    // TCP输出器, 暂未实现
    "tcpOutPut": {
      // 监听地址
      "localAddr": "172.0.0.1",
      // 监听端口
      "port": "9443"
    },
    // UDP输出器
    "udpOutPut": {
      "localAddr": "172.0.0.1",
      "port": "9443"
    }
  },
  "loggers": [
    {
      // logger名称
      "name": "consoleLogger",
      // log级别
      "logLevel": "info",
      // 使能的输出器
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
  // 默认logger
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

## 5. 许可

本项目使用[GPLv3](LICENSE)许可
