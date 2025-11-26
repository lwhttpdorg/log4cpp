# log4cpp

---

中文版本 | [English Education](README.md)

---

## 1. Log4cpp是什么?

log4cpp是一个C++日志库, 参照log4j实现

特性:

- 通过JSON文件配置, 无需修改代码即可改变其行为
- 支持输出日志到STDOUT和STDERR
- 支持输出日志到指定文件
- 支持输出日志到日志服务器(TCP/UDP)
- 单例模式
- 线程安全
- 配置热加载, 修改配置文件无需重启进程就可生效

## 2. 要求

1. 支持C++17及以上的C++编译器
2. CMake 3.11及以上版本
3. nlohmann-json >= 3.7

_警告: 由于MSVC编译器的一些列bug, 本项目不再支持MSVC. 任何MSVC平台的错误都不再解决, 建议使用MingW64_

## 3. 使用

### 3.2. 配置文件

#### 3.2.1. 配置输出格式

```json
{
	"log_pattern": "${NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}"
}
```

说明:

- `${<n>NM}`: logger名称, 如`${8NM}`. `<n>`为logger名长度, 左对齐, 默认是6, 最大为16
- `${yy}`: 2位数表示的年份. 如99, 03
- `${yyyy}`: 完整的年份, 至少4位数, 用'-'表示公元前. 如-0055, 0787, 1999, 2003, 10191
- `${M}`: 数字表示的月份, 无补0. 从1到12
- `${MM}`: 数字表示的月份, 有补0的两位数. 从01到12
- `${MMM}`: 月份的缩写, 3个字母. 从Jan到Dec
- `${d}`: 月份中的第几天, 无补0. 从1到31
- `${dd}`: 月份中的第几天, 有补0的两位数. 从01到31
- `${h}`: 12小时制不补0的小时, AM和PM分别表示上午和下午. 从0到12
- `${hh}`: 12小时制补0的小时. 从00到12
- `${H}`: 24小时制不补0的小时. 从0到23
- `${HH}`: 24小时制补0的小时. 从00到23
- `${m}`: 无补0的分钟. 从1到59
- `${mm}`: 有补0的分钟. 从01到59
- `${s}`: 无补0的秒. 从1到59
- `${ss}`: 有补0的秒. 从01到59
- `${ms}`: 有补0的毫米. 从001到999
- `${<n>TN}`: 线程名, 如`${8TN}`. `<n>`为线程名长度, 左对齐, 默认是16, 最大为16. 如果线程名为空, 使用"T+线程ID"代替, 如"
  main", "T12345"
- `${<n>TH}`: 线程ID, 如`${8TH}`. `<n>`为线程ID位数, 左补0, 默认是8, 最大为8. 如"T12345"
- `${L}`: 日志级别, 取值FATAL, ERROR, WARN, INFO, DEBUG, TRACE
- `${W}`: 日志正文, 如"hello world!"

_注意: 某些系统无法设置线程名, 只能通过线程ID区分多线程_

#### 3.2.2. 配置输出器

配置输出器有四种类型: 控制台输出器(`console`), 文件输出器(`file`), Socket输出器(`socket`, 默认是TCP)

一个简单的配置文件示例:

```json
{
	"appenders": {
		"console": {
			"out_stream": "stdout"
		},
		"file": {
			"file_path": "log/log4cpp.log"
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

#### 3.2.3. 控制台输出器

控制台输出器的作用是将日志输出到STDOUT或STDERR. 典型配置如下:

```json
{
	"appenders": {
		"console": {
			"out_stream": "stdout"
		}
	}
}
```

说明:

- `out_stream`: 输出流, 可以是`stdout`或`stderr`

#### 3.2.4. 文件输出器

文件输出器的作用是将日志输出到指定文件. 典型配置如下:

```json
{
	"appenders": {
		"file": {
			"file_path": "log/log4cpp.log"
		}
	}
}
```

说明:

- `file_path`: 输出文件名

#### 3.2.5. Socket输出器

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

- `addr`: 远端日志服务器地址
- `port`: 远端日志服务器端口
- `protocol`: 协议, 可以是`tcp`或`udp`, 默认是`tcp`
- `prefer-stack`: 优选地址栈, 可以是`IPv4`, `IPv6`, 或者`auto`, 默认是`auto`

_注意: TCP日志服务器如果连接失败, 会采取指数退避重试连接, 直到连接成功_

### 3.3. 配置logger

logger分为命名logger(配置名`loggers`)和默认logger(配置名`root_logger`), `log4cpp::logger_manager::get_logger`
时如果没有指定名称的logger, 则返回默认logger

_注意: 命名logger可以没有, 但是默认logger必须有_

命名logger是一个数组, 每个logger配置包括:

- `name`: logger名称, 用于获取logger, 不能重复, 不能是`root`
- `level`: log级别, 只有大于等于此级别的log才会输出
- `appenders`: 输出器, 只有配置的输出器才会输出. 输出器可以是`console`, `file`, `socket`

默认logger是一个对象, 只有`level`和`appenders`, 没有`name`, 内部实现`name`为`root`

```json
{
	"root": {
		"level": "INFO",
		"appenders": [
			"console",
			"file"
		]
	},
	"loggers": [
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

### 3.4. 加载配置文件

配置文件有两种加载方式:

1. 如果当前路径下存在`log4cpp.json`, 会自动加载此配置文件
2. 如果配置文件不在当前路径下, 或者文件名不是`log4cpp.json`, 需要手动加载配置文件

```c++
const std::string config_file = "log4cpp_config_1.json";
log4cpp::logger_manager &log_mgr = log4cpp::supervisor::get_logger_manager();
log_mgr.load_config(config_file);
```

### 3.5. 配置热加载

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

`SIGHUP`信号会触发`log4cpp`使用之前缓存的路径和文件名重新加载配置文件，重新创建内部对象。先前已经通过
`log4cpp::logger_manager::get_logger()`获得的`std::shared_ptr<log4cpp::log::logger>`
并不会立即失效并且可继续使用，直到最后一个使用者离开其作用域(`std::shared_ptr`引用计数归0)。

_注: `log4cpp::logger_manager::get_logger()`返回的`std::shared_ptr`可能不会发生变化，即使其内部代理对象已经改变_

### 3.6. 在代码中使用

首先需要引入头文件:

```c++
#include <log4cpp/log4cpp.hpp>
```

然后获取logger实例. 通过`name`获取配置logger, 如果不存在指定的logger, 则返回默认的`root_logger`

```c++
std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("aaa");
```

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

- `FATAL`: 致命错误
- `ERROR`: 错误
- `WARN`: 警告
- `INFO`: 信息
- `DEBUG`: 调试
- `TRACE`: 跟踪

### 3.7. 完整示例

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

CMakeLists.txt示例:

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

输出log示例:

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

配置文件实例:

[参考配置文件demo/demo.json](demo/demo.json)

## 4. 构建

欢迎提交PR, 再提交PR之前有些事项需了解:

### 4.1. CMake编译选项

```shell
cmake -S . -B build -DBUILD_LOG4CPP_DEMO=ON -DBUILD_LOG4CPP_TEST=ON -DENABLE_ASAN=ON
```

```shell
cmake --build build --config=Debug -j $(nproc)
```

```shell
ctest --test-dir build --output-on-failure
```

选项说明:

- `-DBUILD_LOG4CPP_DEMO=ON`: 编译demo, 默认不编译
- `-DBUILD_LOG4CPP_TEST=ON`: 编译测试, 默认不开启
- `-DENABLE_ASAN=ON`: 开启地址检测, 默认不开启

### 4.2. 测试

本项目使用Google Test进行单元测试, 测试代码在[test](test)目录下, 欢迎补充测试用例

如果你的代码修改了现有功能, 请确保测试用例覆盖到你的修改

### 4.3. ASAN

如果你的代码修改了现有功能, 请确保ASAN检测通过, 未经ASAN检测通过的代码不会合并

## 5. 许可

本项目使用[LGPLv3](LICENSE)许可
