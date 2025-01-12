# log4cpp

---

中文版本 | [English Education](README.md)

---

## 1. Log4cpp是什么?

log4cpp是一个C++日志库, 参照log4j实现

特性:

* 通过JSON文件配置, 无需修改代码即可改变其行为
* 支持输出日志到STDOUT和STDERR
* 支持输出日志到指定文件
* 支持输出日志到TCP客户端
* 支持输出日志到UDP客户端
* 单例模式
* 线程安全

## 2. 要求

1. 支持C++17及以上的C++编译器
2. CMake 3.11及以上版本
3. Boost >= 1.75

## 3. 使用

### 3.1 在CMake项目中使用

````cmake
include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v3.0.4)

FetchContent_MakeAvailable(log4cpp)

target_link_libraries(${YOUR_TARGET_NAME} log4cpp)
````

### 3.2 配置文件

#### 3.2.1 配置输出格式

```json
{
  "layoutPattern": "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
}
```

说明:

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
* `${W}`: 日志正文, 如"hello world!"

_注意: `${\d+TH}`是一个正则表达式, 用于匹配线程id, 最大宽度为16. 某些系统无法设置线程名, 只能通过线程ID区分多线程_

#### 3.2.2 配置输出器

配置输出器有四种类型: 控制台输出器(console_appender), 文件输出器(file_appender), TCP输出器(tcp_appender), UDP输出器(
udp_appender)

一个简单的配置文件示例:

```json
{
  "appenders": {
    "console_appender": {
      "out_stream": "stdout"
    },
    "file_appender": {
      "file_path": "log/log4cpp.log",
      "append": false
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

#### 3.2.3 控制台输出器

控制台输出器的作用是将日志输出到STDOUT或STDERR. 典型配置如下:

```json
{
  "appenders": {
    "console_appender": {
      "out_stream": "stdout"
    }
  }
}
```

说明:

* `out_stream`: 输出流, 可以是stdout或stderr

#### 3.2.4 文件输出器

文件输出器的作用是将日志输出到指定文件. 典型配置如下:

```json
{
  "appenders": {
    "file_appender": {
      "file_path": "log/log4cpp.log",
      "append": true
    }
  }
}
```

说明:

* `file_path`: 输出文件名
* `append`: 追加还是覆盖, 默认追加(true)

#### 3.2.5 TCP输出器

TCP输出器内部会启动一个TCP服务器, 接受TCP连接, 将日志输出到连接的客户端, 用于输出日志到远程设备. 典型配置如下:

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

说明:

* `local_addr`: 监听地址. 如"0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: 监听端口

_注意: 如果有多个TCP客户端, 会便利所有客户端逐个发送日志_

_注意: 日志为明文传输, 注意隐私和安全问题. 后续不会支持加密传输, 如果需要加密建议先将日志加密后再传递个log4cpp_

#### 3.2.6 UDP输出器

UDP输出器内部会启动一个UDP服务器, 将日志输出到连接的客户端, 用于输出日志到远程设备

与TCP输出器不同, UDP是无连接的, 需要注意:

* UDP是无连接的, 无法保证日志的完整性
* 需要客户端主动发送"hello"消息, 以便服务器获取客户端地址, 以便发送日志
* 客户端退出时需要发送"bye"消息, 以便服务器清理客户端地址, 否则客户端地址会一直保留直到因为日志发送失败而清理或程序退出

典型配置如下:

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

说明:

* `local_addr`: 监听地址. 如"0.0.0.0", "::", "127.0.0.1", "::1"
* `port`: 监听端口

### 3.3 配置logger

logger分为命名logger(配置名`layouts`)和默认logger(配置名`rootLogger`), getLogger时如果没有指定名称的logger, 则返回默认logger

_注意: 命名logger可以没有, 但是默认logger必须有_

命名logger是一个数组, 每个logger配置包括:

* `name`: logger名称, 用于获取logger, 不能重复, 不能是`root`
* `log_level`: log级别, 只有大于等于此级别的log才会输出
* `Appenders`: 输出器, 只有配置的输出器才会输出. 输出器可以是`console_appender`, `file_appender`, `tcp_appender`,
  `udp_appender`

默认logger是一个对象, 只有`log_level`和`Appenders`, 没有`name`, 内部实现`name`为`root`

```json
{
  "layouts": [
    {
      "name": "consoleLogger",
      "log_level": "info",
      "appenders": [
        "console_appender"
      ]
    },
    {
      "name": "recordLogger",
      "log_level": "error",
      "appenders": [
        "file_appender",
        "tcp_appender",
        "udp_appender"
      ]
    }
  ],
  "rootLogger": {
    "log_level": "info",
    "appenders": [
      "file_appender",
      "tcp_appender",
      "udp_appender"
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

然后获取logger实例. 通过`name`获取配置logger, 如果不存在指定的logger, 则返回默认的`rootLogger`

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

说明:

* `FATAL`: 致命错误
* `ERROR`: 错误
* `WARN`: 警告
* `INFO`: 信息
* `DEBUG`: 调试
* `TRACE`: 跟踪

### 3.6 完整示例

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

CMakeLists.txt示例:

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
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG v3.0.4)

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

## 4. 构建

欢迎提交PR, 再提交PR之前有些事项需了解:

### 4.1 boost库

本项目从github在线拉去boost库, 你也可以修改[CMakeLists.txt](src/main/CMakeLists.txt)使用本地boost库,
取消对应位置的注释即可:

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

### 4.2 CMake编译选项

```shell
cmake -S . -B build -DENABLE_DEMO=ON -DENABLE_TESTS=ON -DENABLE_ASAN=ON
cmake --build build --config=Debug
ctest --test-dir build
```

选项说明:

* `-DENABLE_DEMO=ON`: 编译demo, 默认不编译
* `-DENABLE_TEST=ON`: 编译测试, 默认不开启
* `-DENABLE_ASAN=ON`: 开启地址检测, 默认不开启

### 4.3 测试

本项目使用Google Test进行单元测试, 测试代码在[test](src/test)目录下, 欢迎补充测试用例

如果你的代码修改了现有功能, 请确保测试用例覆盖到你的修改

### 4.4 ASAN

如果你的代码修改了现有功能, 请确保ASAN检测通过, 未经ASAN检测通过的代码不会合并

**缺失clang_rt.asan_dynamic-x86_64.dll?**

如果设置了`"ENABLE_ASAN=ON"`且使用的是MSVC编译器, 可能会遇到此问题. 解决办法是:

复制
`"D:\Program Files\Microsoft Visual Studio\<Visual Studio Version>\Professional\VC\Tools\MSVC\<MSVC Version>\bin\Hostx64\x64\clang_rt.asan_dynamic-x86_64.dll"`
到`cmake-build-debug/bin/`

## 5. 许可

本项目使用[LGPLv3](LICENSE)许可
