# log4cpp
## 1. 简述
Logger for C++是一个为C++开发的日志项目, 可以将log输出到控制台或指定的文件
## 2. 特性
- 支持Unix/Linux平台
- 线程安全
## 3. 要求
1. 支持C++11及以上的C++编译器
2. 支持pthread的的编译器
## 3. 使用
### 3.1 编译和安装
```shell
$ cmake -S . -B build -DINSTALL_DEMO=OFF -DCMAKE_INSTALL_PREFIX=/usr/local/log4cpp
$ cd build
$ make
$ sudo make install
```
### 3.2 API
#### a. 头文件
```c++
#include "log4cpp.hpp"
```
#### b. 用法
```c++
// 输出到文件
logger demo_logger("./demo.log", log_level::TRACE);
// 输出到fd, 如stdout
logger demo_logger(STDOUT_FILENO, log_level::WARN);
// 设置每条log的前缀标识, 如果未指定则默认使用线程ID, 即gettid()
demo_logger.set_prefix("demo");
// 输出log
// 输出warning级别的log
demo_logger.log_warn("thread %u: This is a warning...", pthread_self());
// 输出fatal级别的log
demo_logger.log_fatal("This is a fatal...");
// 也可以显示指定log级别
demo_logger.log(log_level::ERROR, "this is a error log");
```
### 3.3 附加说明
#### a. 输出格式
```text
# 年-月-日 时:分:秒 时区 log级别 -- [标识前缀]: log正文
year-mon-day hh:mm:ss timezone log_level -- [ prefix ]: log body
```
其中:  
1. 秒精确到小数点后三位(毫秒)  
2. 标识前缀如果未设置, 默认使用线程ID(通过gettid()获得)
3. log级别的定义如下:
```c++
enum class log_level
{
	FATAL = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4,
	TRACE = 5
};
```
示例:
```text
2022-6-11 22:58:45.869 CST WARN  -- [      8975]: This is a warning...
2022-6-11 22:58:45.869 CST ERROR -- [  thread b]: This is an error...
2022-6-11 22:58:45.869 CST FATAL -- [      8982]: This is a fatal...
```
## 4. 许可
本项目使用[GPLv3](LICENSE)许可