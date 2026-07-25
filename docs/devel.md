# log4cpp Developer Guide

<!-- TOC -->
- [1. Configuring the Build](#1-configuring-the-build)
  - [1.1. CMake](#11-cmake)
    - [1.1.1. Windows](#111-windows)
    - [1.1.2. Linux](#112-linux)
  - [1.2. Meson](#12-meson)
- [2. Building](#2-building)
- [3. Testing](#3-testing)
- [4. Code Coverage](#4-code-coverage)
  - [4.1. CMake](#41-cmake)
  - [4.2. Meson](#42-meson)
- [5. Building RPM/DEB Packages](#5-building-rpmdeb-packages)
  - [5.1. Manual Build](#51-manual-build)
  - [5.2. Using Build Scripts](#52-using-build-scripts)
- [6. Sanitizers](#6-sanitizers)
- [7. Design Documentation](#7-design-documentation)
<!-- /TOC -->

---
[中文版](docs/devel_zh.md) | English Version | [Quick Start](README.md) | [User Guide](docs/user_guide.md)
---

## 1. Configuring the Build

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

Native build:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_LOG4CPP_DEMO=ON -DENABLE_LOG4CPP_UNIT_TEST=ON -DENABLE_ASAN=ON
```

Cross-compilation configuration (for example, ARM64):

```shell
cmake -S . -B cmake-build-debug -DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake
```

CMake options:

* `-DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake`: Use the specified toolchain file for cross-compilation
* `-DCMAKE_BUILD_TYPE=Debug`: Build type, either `Debug` or `Release`; the default is `Release`
* `-DBUILD_LOG4CPP_DEMO=ON`: Build the demo; the default is `OFF`
* `-DENABLE_LOG4CPP_UNIT_TEST=ON`: Build the test programs; the default is `OFF`
* `-DENABLE_ASAN=ON`: Enable AddressSanitizer; the default is `OFF`
* `-DENABLE_LOG4CPP_COVERAGE=ON`: Enable code coverage (GNU only); the default is `OFF`

### 1.2. Meson

Native build:

```shell
meson setup meson-build-debug -Dbuild_demo=true -Denable_tests=true -Db_sanitize=address,undefined
```

Cross-compilation configuration (for example, ARM64):

```shell
meson setup meson-build-debug --cross-file cross/aarch64-linux-gnu.ini
```

Meson options:

* `--cross-file cross/aarch64-linux-gnu.ini`: Use the specified cross-compilation file
* `-Dbuild_demo=true`: Build the demo; the default is `false`
* `-Denable_tests=true`: Build the test programs; the default is `false`
* `-Db_sanitize=address,undefined`: Enable AddressSanitizer and UBSan through Meson's built-in option
* `-Denable_coverage=true`: Enable code coverage (GNU only); the default is `false`

## 2. Building

CMake:

```shell
cmake --build cmake-build-debug -j $(nproc)
```

Meson:

```shell
meson compile -C meson-build-debug -j $(nproc)
```

## 3. Testing

This project uses [Google Test](https://github.com/google/googletest) for unit testing. Tests are located in the
[`test`](test) directory. Changes to existing functionality should include corresponding test coverage.

CMake:

```shell
ctest -C Debug --test-dir cmake-build-debug --output-on-failure
```

For more verbose output:

```shell
ctest -C Debug --test-dir cmake-build-debug --verbose -j $(nproc)
```

Meson:

```shell
meson test -C meson-build-debug --print-errorlogs
```

For more verbose output:

```shell
meson test -C meson-build-debug -v
```

## 4. Code Coverage

Generating a coverage report requires GCC, gcov, and [gcovr](https://gcovr.com/). Run the tests before invoking gcovr
so that the build directory contains coverage data.

gcovr can generate two kinds of HTML report:

* `--html` creates a simple, single-file summary.
* `--html-details` creates an index plus annotated source pages and supporting files. Write it to a dedicated directory
  to keep these files together.

Choose one report type. Both commands use `coverage/index.html` as the entry point and are not intended to be run
together for the same report directory.

The `negative_hits.warn_once_per_file` compatibility option handles negative branch counts that some gcov versions
emit because of a known gcov issue, while still printing one warning for each affected source file.

### 4.1. CMake

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_LOG4CPP_UNIT_TEST=ON -DENABLE_LOG4CPP_COVERAGE=ON
cmake --build cmake-build-debug -j $(nproc)
ctest --test-dir cmake-build-debug --output-on-failure
```

Simple HTML report:

```shell
mkdir -p coverage
gcovr --root . --filter 'src/' \
  --gcov-ignore-parse-errors negative_hits.warn_once_per_file \
  --html coverage/index.html cmake-build-debug
```

Open `coverage/index.html` in a browser.

Detailed HTML report:

```shell
mkdir -p coverage
gcovr --root . --filter 'src/' \
  --gcov-ignore-parse-errors negative_hits.warn_once_per_file \
  --html-details coverage/index.html cmake-build-debug
```

Open `coverage/index.html` in a browser. All files belonging to the detailed report are contained in `coverage`.

### 4.2. Meson

```shell
meson setup meson-build-debug -Dbuildtype=debug -Denable_tests=true -Denable_coverage=true
meson compile -C meson-build-debug -j $(nproc)
meson test -C meson-build-debug --print-errorlogs
```

Simple HTML report:

```shell
mkdir -p coverage
gcovr --root . --filter 'src/' \
  --gcov-ignore-parse-errors negative_hits.warn_once_per_file \
  --html coverage/index.html meson-build-debug
```

Open `coverage/index.html` in a browser.

Detailed HTML report:

```shell
mkdir -p coverage
gcovr --root . --filter 'src/' \
  --gcov-ignore-parse-errors negative_hits.warn_once_per_file \
  --html-details coverage/index.html meson-build-debug
```

Open `coverage/index.html` in a browser. All files belonging to the detailed report are contained in `coverage`.

Use a clean, dedicated build directory when collecting coverage. Stale `.gcda` files from earlier test runs can make
the report inaccurate.

## 5. Building RPM/DEB Packages

### 5.1. Manual Build

Build a DEB package:

```shell
fakeroot debian/rules clean
DEB_BUILD_OPTIONS="noddebs" dpkg-buildpackage -us -uc -b -j$(nproc)
```

Build an RPM package:

```shell
rpmdev-setuptree
VERSION=$(sed -n 's/^project(log4cpp VERSION \([0-9.]*\).*/\1/p' log4cpp/CMakeLists.txt)
tar -czf ~/rpmbuild/SOURCES/liblog4cpp-${VERSION}.tar.gz log4cpp/
sed "s/@VERSION@/${VERSION}/g" log4cpp/liblog4cpp.spec.in > ~/rpmbuild/SPECS/liblog4cpp.spec
rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec
```

The tarball name and spec `Version` come from `liblog4cpp.spec.in` after substituting `@VERSION@`; that value should
match `project(log4cpp VERSION …)` in `CMakeLists.txt` (see `build-rpm.sh`).

### 5.2. Using Build Scripts

The `build-rpm.sh` and `build-deb.sh` scripts build RPM and DEB packages.

For Debian-based systems:

```shell
# Build DEB
./build-deb.sh
# Clean
./build-deb.sh clean
```

For RPM-based systems:

```shell
# Build RPM
./build-rpm.sh
# Clean
./build-rpm.sh clean
```

Options:

* `clean`: Clean build artifacts, including generated tarballs, spec files, and built packages
* `-a, --arch <ARCH>`: Specify the target package architecture, such as `amd64` or `arm64`; the default is the host
  architecture

## 6. Sanitizers

Changes to existing functionality must pass AddressSanitizer checks.

CMake:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_LOG4CPP_UNIT_TEST=ON
```

Meson (using the built-in `b_sanitize` option):

```shell
meson setup meson-build-debug -Denable_tests=true -Db_sanitize=address,undefined
```

## 7. Design Documentation

See the [design document](docs/design.md) for the architecture, core classes, data flow, thread-safety strategy, and
extension points.
