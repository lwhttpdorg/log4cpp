# log4cpp
## 简述
Logger for C++是一个为C++开发的日志项目, 可以将log输出到控制台或指定的文件  
输出的格式如下:
```text
year-mon-day hh:mm:ss timezone log_level -- [ prefix ]: log body
```
例如:
```text
2022-6-5 19:28:05.211 CST WARN  -- [    main]: I am main  thread 4130355008
2022-6-5 19:28:05.212 CST ERROR -- [        ]: I am child thread 4121958144
2022-6-5 19:28:05.212 CST FATAL -- [   child]: I am child thread 4130350848
```
## 特性
 - 仅支持Unix/Linux平台
## 使用
1. ### 编译安装
```shell
$ cmake -S . -B build -DINSTALL_DEMO=ON
$ cd build
$ make
$ sudo make install
```
1. ### 示例
头文件:
```c++
#include "log4cpp.hpp"
```
用法:
```c++
// 输出到文件
logger demo_logger("./demo.log", log_level::TRACE);
// 输出到fd, 如stdout
logger demo_logger(STDOUT_FILENO, log_level::WARN);
// 设置每条log的前缀
demo_logger.set_log_prefix("demo");
// 输出一条log
demo_logger.log_warn("thread %u: this is a warnning log", pthread_self());
demo_logger.log_fatal("this is a warnning log");
demo_logger.log(log_level::ERROR, "this is a error log");
```
## 许可
本项目使用GPLv3许可