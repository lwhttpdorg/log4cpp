# log4cpp
## 1. 简述
Logger for C++是一个为C++开发的日志项目, 可以将log输出到控制台或指定的文件
## 2. 特性
- 支持Unix/Linux平台
- 线程安全
## 3. 要求
1. 支持C++11及以上的C++编译器
2. 支持pthread的的编译器
3. CMake 3.11及以上版本
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
#include "log4cpp.hpp"

int main()
{
	// 设置YAML路径, 可选. 默认使用当前路径下的log4cpp.yml
	LoggerManager::setYamlFilePath("./log4cpp.yml");
	Logger logger = LoggerManager::getLogger("main");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);

	logger = LoggerManager::getLogger("test");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);
	return 0;
}
```
### 3.3 附加说明
#### a. 输出格式
```text
# 年-月-日 时:分:秒 时区 log级别 -- [标识前缀]: log正文
year-mon-day hh:mm:ss timezone LogLevel -- [ prefix ]: log body
```
其中:  
1. 秒精确到小数点后三位(毫秒)  
2. 标识前缀如果未设置, 默认使用线程ID(通过gettid()获得)
3. log级别的定义如下:
```c++
enum class LogLevel
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
## 4. YAML示例
```yaml
log4cpp:
  # 输出器, 至少要有一个
  outputters:
    # 控制台输出器
    consoleOutputter:
      # 级别
      logLevel: trace
    # 文件输出器
    fileOutputter:
      # 输出文件路径
      filePath: "/home/nereus/WorkSpace/log4cpp/log/log4cpp.log"
      # 异步输出, 默认开启, 可提高性能(暂未实现)
      async: true
      # 追加而不是覆盖, 默认追加(暂未实现)
      append: false
  # 日志记录器
  loggers:
    - name: main      # 名称
      logLevel: trace # 级别
      outputter: # 输出器, 可配置多个, 见outputters
        - consoleOutputter

    - name: test
      logLevel: error
      outputter:
        - consoleOutputter
        - fileOutputter
  # 根日志记录器, 如果loggers中没有定义, 则使用此配置
  root:
    pattern: 1     # 输出格式表达式(暂未实现)
    logLevel: info # 级别
    outputter: # 输出器
      - consoleOutputter
      - fileOutputter
```
## 5. 许可
本项目使用[GPLv3](LICENSE)许可
