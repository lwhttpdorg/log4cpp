# log4cpp

<!-- TOC -->
- [1. Log4cpp是什么?](#1-log4cpp%E6%98%AF%E4%BB%80%E4%B9%88)
- [2. 要求](#2-%E8%A6%81%E6%B1%82)
- [3. 使用](#3-%E4%BD%BF%E7%94%A8)
  - [3.1. 快速入门](#31-%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8)
    - [3.1.1. 创建项目](#311-%E5%88%9B%E5%BB%BA%E9%A1%B9%E7%9B%AE)
      - [3.1.1.1. CMake](#3111-cmake)
      - [3.1.1.2. Meson](#3112-meson)
    - [3.1.2. 引入头文件](#312-%E5%BC%95%E5%85%A5%E5%A4%B4%E6%96%87%E4%BB%B6)
    - [3.1.3. 加载配置文件(可选)](#313-%E5%8A%A0%E8%BD%BD%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6%E5%8F%AF%E9%80%89)
    - [3.1.4. 获取logger](#314-%E8%8E%B7%E5%8F%96logger)
    - [3.1.5. 输出log](#315-%E8%BE%93%E5%87%BAlog)
    - [3.1.6. 在类中使用](#316-%E5%9C%A8%E7%B1%BB%E4%B8%AD%E4%BD%BF%E7%94%A8)
    - [3.1.7. 完整示例](#317-%E5%AE%8C%E6%95%B4%E7%A4%BA%E4%BE%8B)
    - [3.1.8. 配置文件](#318-%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6)
    - [3.1.9. 构建并运行](#319-%E6%9E%84%E5%BB%BA%E5%B9%B6%E8%BF%90%E8%A1%8C)
- [4. 许可](#4-%E8%AE%B8%E5%8F%AF)
<!-- /TOC -->

---
中文版 | [English Version](README.md) | [用户指南](docs/user_guide_zh.md) | [开发者指南](docs/devel_zh.md)
---

## 1. Log4cpp是什么?

log4cpp是一个C++日志库, 参照log4j实现

特性:

* 通过JSON文件配置, 无需修改代码即可改变其行为
* 支持输出日志到STDOUT和STDERR
* 支持输出日志到指定文件
* 支持文件日志按大小、时间、启动滚动, 并支持按数量和历史时间清理归档
* 支持输出日志到日志服务器(TCP/UDP)
* 单例模式
* 线程安全
* 配置热加载, 修改配置文件无需重启进程就可生效

## 2. 要求

1. 支持C++20及以上的C++编译器
2. CMake 3.10及以上版本 (CMake构建)
3. Meson 1.1.0及以上版本 (Meson构建)

## 3. 使用

### 3.1. 快速入门

创建下面的项目:

```text
log4cpp-demo/
├── CMakeLists.txt
├── meson.build
├── subprojects/
│   └── log4cpp.wrap
├── demo.cpp
└── log4cpp.json（可选）
```

两种构建系统共用同一份`demo.cpp`和可选的`log4cpp.json`。项目可以只保留其中一种构建定义，也可以
同时支持CMake和Meson。

#### 3.1.1. 创建项目

##### 3.1.1.1. CMake

使用`FetchContent`:

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

或者使用`pkg-config`(已经安装了log4cpp的deb/rpm包):

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

将下面的内容保存为`meson.build`:

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

如果尚未安装log4cpp，创建`subprojects/log4cpp.wrap`，Meson即可自动拉取源码:

```ini
[wrap-git]
directory = log4cpp
url = https://github.com/lwhttpdorg/log4cpp.git
revision = v5.0.0
depth = 1
```

#### 3.1.2. 引入头文件

头文件:

```c++
#include <log4cpp/log4cpp.hpp>
```

#### 3.1.3. 加载配置文件(可选)

配置文件有两种加载方式:

1. 如果当前路径下存在`log4cpp.json`, 会自动加载此配置文件
2. 如果配置文件不在当前路径下, 或者文件名不是`log4cpp.json`, 需要手动加载配置文件
3. 如果不存在`log4cpp.json`, 也没有手动加载其他配置, 会使用内置默认配置

```c++
const std::string config_file = "demo.json";
auto &log_mgr = log4cpp::supervisor::get_logger_manager();
log_mgr.load_config(config_file);
```

#### 3.1.4. 获取logger

通过`name`获取配置logger

```c++
std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger(const std::string &name = "root");
```

你可以指定一个唯一的字符串, 它可以输出到log中(通过"log-pattern"中的`${<n>NM}`可以指定输出时的长度)

```shell
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
```

#### 3.1.5. 输出log

获取logger后, 可以使用下面的方法输出log:

```c++
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

上面的方法内部调用了下面的方法, 也可以直接调用下面的方法:

```c++
void log(log_level level, std::string_view msg);
template <class... Args> void log(log_level level, std::format_string<Args...> fmt, Args &&...args);
```

格式化日志使用C++20 `std::format`语法:

```c++
logger->info("user={}, cost={}ms", user_name, cost_ms);
```

如果希望日志消息中包含源码文件名和行号, 可以使用`at()`:

```c++
logger->at().info("user={}, cost={}ms", user_name, cost_ms);
```

其中log级别`log_level level`的定义如下:

```c++
namespace log4cpp {
  enum class log_level { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };
}
```

说明:

* `FATAL`: 致命错误
* `ERROR`: 错误
* `WARN`: 警告
* `INFO`: 信息
* `DEBUG`: 调试
* `TRACE`: 跟踪

#### 3.1.6. 在类中使用

还可以将logger对象作为类成员变量(或者是静态成员变量), 因为是`std::shared_ptr`该类的所有实例都使用同一个`logger`

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

你将得到下面的log:

```shell
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- constructor
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- func(hello)
demo: 2025-11-29 20:06:47:652 [main  ] [INFO ] -- destructor
```

#### 3.1.7. 完整示例

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

使用下方可选配置时的log输出示例:

```shell
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
hello  : 2025-11-13 23:32:02:475 [main  ] [FATAL] -- this is a fatal
root   : 2025-11-13 23:32:02:475 [child   ] [INFO ] -- this is an info
hello  : 2025-11-13 23:32:02:475 [main  ] [INFO ] -- this is an info
root   : 2025-11-13 23:32:02:475 [child   ] [WARN ] -- this is a warning
hello  : 2025-11-13 23:32:02:475 [main  ] [WARN ] -- this is a warning
root   : 2025-11-13 23:32:02:475 [child   ] [ERROR] -- this is an error
hello  : 2025-11-13 23:32:02:475 [main  ] [ERROR] -- this is an error
root   : 2025-11-13 23:32:02:475 [child   ] [FATAL] -- this is a fatal
```

#### 3.1.8. 配置文件

配置文件是可选的。没有配置文件时，log4cpp会使用内置日志格式、输出到`stdout`，并使用日志级别为
`WARN`的`root` logger。

如需自定义，将下面的内容保存为`log4cpp.json`。log4cpp会自动从当前工作目录加载此文件；该配置包含
控制台和文件输出、日志格式及具名logger:

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

仓库中的[demo配置](demo/demo.json)还包含Socket输出器，可作为更完整的参考。

#### 3.1.9. 构建并运行

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

在Windows上运行生成的`demo.exe`。使用支持多配置的CMake生成器时，通过`--config Release`构建，并在
`build`目录中运行`Release\demo.exe`。

配置格式、输出器、滚动策略、logger继承及配置热加载请参阅[用户指南](docs/user_guide_zh.md)。

## 4. 许可

本项目使用[LGPLv3](LICENSE)许可
