# log4cpp

## 1. 简述

log4cpp是一个简单的C++日志库, 支持多线程, 支持自定义输出格式, 支持配置文件, 支持控制台, 文件, TCP, UDP输出

## 2. 要求

1. 支持C++17及以上的C++编译器
2. CMake 3.11及以上版本
3. Boost >= 1.75

## 3. 使用

### 3.1 在CMake项目中使用

````cmake
include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v2.0.1)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${YOUR_TARGET_NAME} log4cpp)
````

### 3.2 配置文件

#### 3.2.1 配置输出格式

```json
{
  "pattern": "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
}
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
* `${ms}`: Milliseconds with leading zeros. 001 to 999
* `${TH}`: The name of the thread, if the name is empty, use thread id instead, e.g. T12345
* `${\d+TH}`: The regular expression to match the thread id pattern, e.g. ${8TH}. max width is 16. if the name is empty,
  use thread id instead, e.g. T12345
* `${L}`: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${W}`: Log content, Examples: hello world!

_注意: `${\d+TH}`是一个正则表达式, 用于匹配线程id, 最大宽度为16. 某些系统无法设置线程名, 只能通过线程ID区分多线程_

#### 3.2.2 配置输出器

配置输出器有四种类型: 控制台输出器(consoleOutPut), 文件输出器(fileOutPut), TCP输出器(tcpOutPut), UDP输出器(udpOutPut)

一个简单的配置文件示例:

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

#### 3.2.3 控制台输出器

控制台输出器的作用是将日志输出到STDOUT或STDERR. 典型配置如下:

```json
{
  "logOutPut": {
    "consoleOutPut": {
      "outStream": "stdout"
    }
  }
}
```

解释:

* `outStream`: 输出流, 可以是stdout或stderr

#### 3.2.4 文件输出器

文件输出器的作用是将日志输出到指定文件. 典型配置如下:

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

解释:

* `filePath`: 输出文件名
* `append`: 追加还是覆盖, 默认追加(true)

#### 3.2.5 TCP输出器

TCP输出器内部会启动一个TCP服务器, 接受TCP连接, 将日志输出到连接的客户端, 用于输出日志到远程设备. 典型配置如下:

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

解释:

* `localAddr`: 监听地址. 如"0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: 监听端口

_注意: 如果有多个TCP客户端, 会便利所有客户端逐个发送日志_

_注意: 日志为明文传输, 注意隐私和安全问题. 后续不会支持加密传输, 如果需要加密建议先将日志加密后再传递个log4cpp_

#### 3.2.6 UDP输出器

UDP输出器内部会启动一个UDP服务器, 将日志输出到连接的客户端, 用于输出日志到远程设备

与TCP输出器不同, UDP是无连接的, 需要注意:

* UDP是无连接的, 无法保证日志的完整性
* 需要客户端主动发送"hello"消息, 以便服务器获取客户端地址, 以便发送日志
* 客户端退出时需要发送"goodbye"消息, 以便服务器清理客户端地址, 否则客户端地址会一直保留直到因为日志发送失败而清理或程序退出

典型配置如下:

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

解释:

* `localAddr`: 监听地址. 如"0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: 监听端口

### 3.3 配置logger

logger分为命名logger(配置名`loggers`)和默认logger(配置名`rootLogger`), getLogger时如果没有指定名称的logger, 则返回默认logger

_注意: 命名logger可以没有, 但是默认logger必须有_

命名logger是一个数组, 每个logger配置包括:

* `name`: logger名称, 用于获取logger, 不能重复, 不能是`root`
* `logLevel`: log级别, 只有大于等于此级别的log才会输出
* `logOutPuts`: 输出器, 只有配置的输出器才会输出. 输出器可以是`consoleOutPut`, `fileOutPut`, `tcpOutPut`, `udpOutPut`

默认logger是一个对象, 只有`logLevel`和`logOutPuts`, 没有`name`, 内部实现`name`为`root`

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

### 3.4 加载配置文件

配置文件有两种加载方式:

1. 如果当前路径下存在`log4cpp.json`, 会自动加载此配置文件
2. 如果配置文件不在当前路径下, 或者文件名不是`log4cpp.json`, 需要手动加载配置文件

```c++
log4cpp::logger_manager::load_config("/config_path/log4cpp.json");
```

### 3.5 在代码中使用

首先需要引入头文件:

```c++
#include "log4cpp.hpp"
```

然后获取logger实例. 通过`name`获取配置的`"name": "consoleLogger"`logger, 如果不存在指定的logger,
则返回默认的`rootLogger`

```c++
std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
```

获取logger后, 可以使用下面的方法输出log:

```c++
void trace(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void warn(const char *__restrict fmt, ...);
void error(const char *__restrict fmt, ...);
void fatal(const char *__restrict fmt, ...);
```

上面的方法内部调用了下面的方法, 也可以直接调用下面的方法:

```c++
void log(log_level level, const char *fmt, ...);
```

其中log级别`log_level level`的定义如下:

```c++
namespace log4cpp {
  enum class log_level {
    FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
  };
}
```

解释:

* `FATAL`: 致命错误
* `ERROR`: 错误
* `WARN`: 警告
* `INFO`: 信息
* `DEBUG`: 调试
* `TRACE`: 跟踪

### 3.6 完整示例

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

CMakeLists.txt示例:

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

输出log示例:

```shell
2025-01-02 22:53:04:329 [    main] [INFO ] -- this is a info
2025-01-02 22:53:04:329 [   child] [ERROR] -- this is an error
2025-01-02 22:53:04:329 [    main] [WARN ] -- this is an warning
2025-01-02 22:53:04:329 [   child] [FATAL] -- this is a fatal
2025-01-02 22:53:04:329 [    main] [ERROR] -- this is an error
2025-01-02 22:53:04:329 [    main] [FATAL] -- this is a fatal
```

配置文件实例:

[参考配置文件示例](demo/log4cpp.json)

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

### 3.7 贡献

欢迎提交PR, 再提交PR之前有些事项需了解:

#### 3.7.1 boost库

本项目从github在线拉去boost库, 你也可以修改[CMakeLists.txt](src/main/CMakeLists.txt)使用本地boost库,
取消对应位置的注释即可:

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

如果CMake没有自动找到Boost路径, 可以手动设置Boost路径:

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

#### 3.7.2 CMake编译选项

```shell
$ cmake -S . -B build -DENABLE_DEMO=ON
```

选项说明:

* `-DENABLE_DEMO=ON`: 编译demo, 默认不编译
* `-DENABLE_TEST=ON`: 编译测试, 默认不开启
* `-DENABLE_ASAN=ON`: 开启地址检测, 默认不开启

#### 3.7.3 测试

本项目使用Google Test进行单元测试, 测试代码在[test](src/test)目录下, 欢迎补充测试用例

如果你的代码修改了现有功能, 请确保测试用例覆盖到你的修改

#### 3.7.4 ASAN

如果你的代码修改了现有功能, 请确保ASAN检测通过, 未经ASAN检测通过的代码不会合并

## 4. 许可

本项目使用[GPLv3](LICENSE)许可
