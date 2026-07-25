# log4cpp 开发者指南

<!-- TOC -->
- [1. 配置构建](#1-%E9%85%8D%E7%BD%AE%E6%9E%84%E5%BB%BA)
  - [1.1. CMake](#11-cmake)
    - [1.1.1. Windows](#111-windows)
    - [1.1.2. Linux](#112-linux)
  - [1.2. Meson](#12-meson)
- [2. 构建](#2-%E6%9E%84%E5%BB%BA)
- [3. 测试](#3-%E6%B5%8B%E8%AF%95)
- [4. 代码覆盖率](#4-%E4%BB%A3%E7%A0%81%E8%A6%86%E7%9B%96%E7%8E%87)
  - [4.1. CMake](#41-cmake)
  - [4.2. Meson](#42-meson)
- [5. 构建RPM/DEB包](#5-%E6%9E%84%E5%BB%BArpmdeb%E5%8C%85)
  - [5.1. 手动构建](#51-%E6%89%8B%E5%8A%A8%E6%9E%84%E5%BB%BA)
  - [5.2. 使用构建脚本](#52-%E4%BD%BF%E7%94%A8%E6%9E%84%E5%BB%BA%E8%84%9A%E6%9C%AC)
- [6. Sanitizer检查](#6-sanitizer%E6%A3%80%E6%9F%A5)
- [7. 设计文档](#7-%E8%AE%BE%E8%AE%A1%E6%96%87%E6%A1%A3)
<!-- /TOC -->

---
中文版 | [English Version](devel.md) | [快速入门](README_ZH.md) | [用户指南](user_guide_zh.md)
---

## 1. 配置构建

### 1.1. CMake

#### 1.1.1. Windows

MinGW64:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DENABLE_LOG4CPP_UNIT_TEST=ON -G "MinGW Makefiles"
```

MSVC:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DENABLE_LOG4CPP_UNIT_TEST=ON -G "Visual Studio 17 2022" -A x64
```

#### 1.1.2. Linux

原生构建:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DENABLE_LOG4CPP_UNIT_TEST=ON -DENABLE_ASAN=ON
```

交叉编译（以ARM64为例）:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake
```

CMake选项:

* `-DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake`: 指定交叉编译所使用的toolchain文件
* `-DCMAKE_BUILD_TYPE=Debug`: 构建类型，可选`Debug`或`Release`，默认是`Release`
* `-DBUILD_LOG4CPP_DEMO=ON`: 编译demo，默认是`OFF`
* `-DENABLE_LOG4CPP_UNIT_TEST=ON`: 编译测试程序，默认是`OFF`
* `-DENABLE_ASAN=ON`: 启用AddressSanitizer，默认是`OFF`
* `-DENABLE_LOG4CPP_COVERAGE=ON`: 启用代码覆盖率（仅GNU），默认是`OFF`

### 1.2. Meson

原生构建:

```shell
meson setup meson-build-debug -Dbuild_demo=true -Denable_tests=true -Db_sanitize=address,undefined
```

交叉编译（以ARM64为例）:

```shell
meson setup meson-build-debug --cross-file cross/aarch64-linux-gnu.ini
```

Meson选项:

* `--cross-file cross/aarch64-linux-gnu.ini`: 指定交叉编译所使用的文件
* `-Dbuild_demo=true`: 编译demo，默认是`false`
* `-Denable_tests=true`: 编译测试程序，默认是`false`
* `-Db_sanitize=address,undefined`: 通过Meson内置选项启用AddressSanitizer和UBSan
* `-Denable_coverage=true`: 启用代码覆盖率（仅GNU），默认是`false`

## 2. 构建

CMake:

```shell
cmake --build cmake-build-debug -j $(nproc)
```

Meson:

```shell
meson compile -C meson-build-debug -j $(nproc)
```

## 3. 测试

本项目使用[Google Test](https://github.com/google/googletest)进行单元测试，测试代码位于[`test`](test)目录。
修改现有功能时，请添加覆盖相应改动的测试用例。

CMake:

```shell
ctest -C Debug --test-dir cmake-build-debug --output-on-failure
```

输出更详细的信息:

```shell
ctest -C Debug --test-dir cmake-build-debug --verbose -j $(nproc)
```

Meson:

```shell
meson test -C meson-build-debug --print-errorlogs
```

输出更详细的信息:

```shell
meson test -C meson-build-debug -v
```

## 4. 代码覆盖率

生成覆盖率报告需要GCC、gcov和[gcovr](https://gcovr.com/)。调用gcovr之前应先运行测试，使构建目录中
生成覆盖率数据。

下面的命令会生成包含各源码文件明细的HTML报告。完成后使用浏览器打开`coverage.html`即可查看。

### 4.1. CMake

```shell
cmake -S . -B cmake-build-coverage -DCMAKE_BUILD_TYPE=Debug -DENABLE_LOG4CPP_UNIT_TEST=ON -DENABLE_LOG4CPP_COVERAGE=ON
cmake --build cmake-build-coverage -j $(nproc)
ctest --test-dir cmake-build-coverage --output-on-failure
gcovr --root . --filter 'src/' --html-details coverage.html cmake-build-coverage
```

### 4.2. Meson

```shell
meson setup meson-build-coverage -Dbuildtype=debug -Denable_tests=true -Denable_coverage=true
meson compile -C meson-build-coverage -j $(nproc)
meson test -C meson-build-coverage --print-errorlogs
gcovr --root . --filter 'src/' --html-details coverage.html meson-build-coverage
```

收集覆盖率时建议使用干净的专用构建目录。之前测试遗留的`.gcda`文件可能导致报告结果不准确。

## 5. 构建RPM/DEB包

### 5.1. 手动构建

构建DEB:

```shell
fakeroot debian/rules clean
DEB_BUILD_OPTIONS="noddebs" dpkg-buildpackage -us -uc -b -j$(nproc)
```

构建RPM:

```shell
rpmdev-setuptree
VERSION=$(sed -n 's/^project(log4cpp VERSION \([0-9.]*\).*/\1/p' log4cpp/CMakeLists.txt)
tar -czf ~/rpmbuild/SOURCES/liblog4cpp-${VERSION}.tar.gz log4cpp/
sed "s/@VERSION@/${VERSION}/g" log4cpp/liblog4cpp.spec.in > ~/rpmbuild/SPECS/liblog4cpp.spec
rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec
```

源码包名称与spec中的`Version`由`liblog4cpp.spec.in`经`@VERSION@`替换得到，应与`CMakeLists.txt`中的
`project(log4cpp VERSION …)`一致（也可直接使用`build-rpm.sh`）。

### 5.2. 使用构建脚本

本项目提供`build-rpm.sh`和`build-deb.sh`脚本，用于构建RPM和DEB软件包。

基于Debian的发行版:

```shell
# 构建DEB
./build-deb.sh
# 清理
./build-deb.sh clean
```

基于RPM的发行版:

```shell
# 构建RPM
./build-rpm.sh
# 清理
./build-rpm.sh clean
```

选项:

* `clean`: 清理构建产物，包括生成的tar包、spec文件和软件包
* `-a, --arch <ARCH>`: 指定软件包的目标架构，如`amd64`、`arm64`，默认为主机架构

## 6. Sanitizer检查

修改现有功能时，请确保AddressSanitizer检查通过。

CMake:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_LOG4CPP_UNIT_TEST=ON
```

Meson（使用内置的`b_sanitize`选项）:

```shell
meson setup meson-build-debug -Denable_tests=true -Db_sanitize=address,undefined
```

## 7. 设计文档

架构、核心类、数据流、线程安全策略和扩展点请参阅[设计文档](docs/design.md)。
