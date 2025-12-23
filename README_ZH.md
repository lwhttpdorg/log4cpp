# log4cpp

---

中文版本 | [English Version](README.md)

---

<!-- TOC -->
* [log4cpp](#log4cpp)
  * [1. Log4cpp是什么?](#1-log4cpp是什么)
  * [2. 要求](#2-要求)
  * [3. 使用](#3-使用)
    * [3.1. 快速入门](#31-快速入门)
      * [3.1.1. 创建CMake项目](#311-创建cmake项目)
      * [3.1.2. 引入头文件](#312-引入头文件)
      * [3.1.3. 加载配置文件(可选)](#313-加载配置文件可选)
      * [3.1.4. 获取logger](#314-获取logger)
      * [3.1.5. 输出log](#315-输出log)
      * [3.1.6. 在类中使用](#316-在类中使用)
      * [3.1.7. 完整示例](#317-完整示例)
    * [3.2. 进阶用法](#32-进阶用法)
      * [3.2.1. 配置文件](#321-配置文件)
        * [3.2.1.1. 输出格式](#3211-输出格式)
        * [3.2.1.2. 输出器(Appender)](#3212-输出器appender)
        * [3.2.1.3. 控制台输出器](#3213-控制台输出器)
        * [3.2.1.4. 文件输出器](#3214-文件输出器)
        * [3.2.1.5. Socket输出器](#3215-socket输出器)
        * [3.2.1.6. logger](#3216-logger)
    * [3.3. 配置热加载](#33-配置热加载)
  * [4. 构建](#4-构建)
    * [4.1. 配置](#41-配置)
      * [4.1.1. Windows](#411-windows)
      * [4.1.2. Linux](#412-linux)
    * [4.2. 构建](#42-构建)
    * [4.3. 测试](#43-测试)
    * [4.4. ASAN](#44-asan)
  * [5. 许可](#5-许可)
<!-- TOC -->

## 1. Log4cpp是什么?

log4cpp是一个C++日志库, 参照log4j实现

特性:

* 通过JSON文件配置, 无需修改代码即可改变其行为
* 支持输出日志到STDOUT和STDERR
* 支持输出日志到指定文件
* 支持输出日志到日志服务器(TCP/UDP)
* 单例模式
* 线程安全
* 配置热加载, 修改配置文件无需重启进程就可生效

## 2. 要求

1. 支持C++17及以上的C++编译器
2. CMake 3.11及以上版本
3. nlohmann-json >= 3.0

## 3. 使用

### 3.1. 快速入门

#### 3.1.1. 创建CMake项目

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.11)

project(log4cpp-demo)

add_executable(demo main.cpp)

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/lwhttpdorg/log4cpp.git GIT_TAG v4.0.3)
FetchContent_MakeAvailable(log4cpp)
target_link_libraries(demo log4cpp)
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
3. 如果下存在`log4cpp.json`, 也没有手动加载, 会使用内置的默认配置

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
void trace(const char *__restrict fmt, ...);
void debug(const char *__restrict fmt, ...);
void info(const char *__restrict fmt, ...);
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
        logger->info("func(%s)", name.c_str());
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

输出log示例:

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

配置文件实例:

[参考配置文件demo/demo.json](demo/demo.json)

### 3.2. 进阶用法

#### 3.2.1. 配置文件

##### 3.2.1.1. 输出格式

```json
{
	"log-pattern": "${NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${msg}"
}
```

说明:

* `${<n>NM}`: logger名称, 如`${8NM}`. `<n>`为logger名长度, 左对齐, 默认是6, 最大为64
* `${yy}`: 2位数表示的年份. 如99, 03
* `${yyyy}`: 完整的年份, 至少4位数, 用'-'表示公元前. 如-0055, 0787, 1999, 2003, 10191
* `${M}`: 数字表示的月份, 无补0. 从1到12
* `${MM}`: 数字表示的月份, 有补0的两位数. 从01到12
* `${MMM}`: 月份的缩写, 3个字母. 从Jan到Dec
* `${d}`: 月份中的第几天, 无补0. 从1到31
* `${dd}`: 月份中的第几天, 有补0的两位数. 从01到31
* `${h}`: 12小时制不补0的小时, AM和PM分别表示上午和下午. 从0到12
* `${hh}`: 12小时制补0的小时. 从00到12
* `${H}`: 24小时制不补0的小时. 从0到23
* `${HH}`: 24小时制补0的小时. 从00到23
* `${m}`: 无补0的分钟. 从1到59
* `${mm}`: 有补0的分钟. 从01到59
* `${s}`: 无补0的秒. 从1到59
* `${ss}`: 有补0的秒. 从01到59
* `${ms}`: 有补0的毫米. 从001到999
* `${<n>TN}`: 线程名, 如`${8TN}`. `<n>`为线程名长度, 左对齐, 默认是16, 最大为16. 如果线程名为空, 使用"T+线程ID"代替, 如"
  main", "T12345"
* `${<n>TH}`: 线程ID, 如`${8TH}`. `<n>`为线程ID位数, 左补0, 默认是8, 最大为8. 如"T12345"
* `${L}`: 日志级别, 取值FATAL, ERROR, WARN, INFO, DEBUG, TRACE
* `${msg}`: 日志消息, 如"hello world!"

_注意: 某些系统无法设置线程名, 只能通过线程ID区分多线程_

_注: 默认log-pattern为`"${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${msg}"`_

##### 3.2.1.2. 输出器(Appender)

输出器有四种类型: 控制台输出器(`console`), 文件输出器(`file`), Socket输出器(`socket`, 默认是TCP)

一个简单的配置文件示例:

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

##### 3.2.1.3. 控制台输出器

控制台输出器的作用是将日志输出到STDOUT或STDERR. 典型配置如下:

```json
{
	"appenders": {
		"console": {
			"out-stream": "stdout"
		}
	}
}
```

说明:

* `out-stream`: 输出流, 可以是`stdout`或`stderr`

##### 3.2.1.4. 文件输出器

文件输出器的作用是将日志输出到指定文件. 典型配置如下:

```json
{
	"appenders": {
		"file": {
			"file-path": "log/log4cpp.log"
		}
	}
}
```

说明:

* `file-path`: 输出文件名

##### 3.2.1.5. Socket输出器

Socket输出器支持TCP和UDP两种协议, 通过`protocol`字段区分, 如果不配置`protocol`, 则默认是TCP

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

说明:

* `host`: 远端日志服务器hostname
* `port`: 远端日志服务器端口
* `protocol`: 协议, 可以是`tcp`或`udp`, 默认是`tcp`
* `prefer-stack`: 优选地址栈, 可以是`IPv4`, `IPv6`, 或者`auto`, 默认是`auto`

_注意: TCP日志服务器如果连接失败, 会采取指数退避重试连接, 直到连接成功_

##### 3.2.1.6. logger

`loggers`是一个数组, 每个logger配置包括:

* `name`: logger名称, 用于获取logger, 不能重复. `root`为默认logger
* `level`: log级别, 只有大于等于此级别的log才会输出, 非`root`可以省略(自动继承`root`)
* `appenders`: 输出器, 只有配置的输出器才会输出. 输出器可以是`console`, `file`, `socket`. 非`root`可以省略(自动继承
  `root`)

__注: 必须定义`name`为`root`默认logger__

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

### 3.3. 配置热加载

配置热加载可以实现修改配置文件后，不重启进程就能使配置生效(仅支持Linux系统)

_注: 配置文件路径和名称不能变化，使用启动时的路径和名称加载_

首先需要使能配置热加载:

```c++
log4cpp::supervisor::enable_config_hot_loading(int sig = SIGHUP);
```

修改配置文件后，向你的进程发送信号(默认是`SIGHUP`):

```shell
kill -SIGHUP <PID>
```

此操作会触发`log4cpp`使用之前缓存的路径和文件名重新加载配置文件，重新创建内部对象。先前已经通过`log4cpp::logger_manager::get_logger()`获得的`std::shared_ptr<log4cpp::logger>`
并不会立即失效并且可继续使用
此操作会触发`log4cpp`使用之前缓存的路径和文件名重新加载配置文件。由于内部采用了代理模式，您之前通过 `get_logger()` 获取的 `std::shared_ptr` 依然有效且可以继续使用，它会自动将日志记录请求转发到新的、基于新配置的内部实现上。

_注: `log4cpp::logger_manager::get_logger()`返回的`std::shared_ptr`可能不会发生变化，即使其内部代理对象已经改变_

## 4. 构建

### 4.1. 配置

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

选项:

* `-DBUILD_LOG4CPP_DEMO=ON`: 编译demo, 默认`OFF`不编译
* `-DBUILD_LOG4CPP_TEST=ON`: 编译测试, 默认`OFF`不开启
* `-DENABLE_ASAN=ON`: 开启地址检测, 默认`OFF`不开启

### 4.2. 构建

```shell
cmake --build build -j $(nproc)
```

### 4.3. 测试

本项目使用Google Test进行单元测试, 测试代码在[test](test)目录下, 欢迎补充测试用例

如果你的代码修改了现有功能, 请确保测试用例覆盖到你的修改

```shell
ctest -C Debug --test-dir build --output-on-failure
```

或者输出详细信息:

```shell
ctest -C Debug --test-dir build --verbose
```

### 4.4. ASAN

如果你的代码修改了现有功能, 请确保ASAN检测通过, 未经ASAN检测通过的代码不会合并

## 5. 许可

本项目使用[LGPLv3](LICENSE)许可

