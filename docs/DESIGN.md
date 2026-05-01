# log4cpp Design Document


<!-- TOC -->
- [1. Overview](#1.-overview)
  - [1.1. Key Features](#1.1.-key-features)
- [2. Architecture](#2.-architecture)
  - [2.1. System Architecture Diagram](#2.1.-system-architecture-diagram)
  - [2.2. Component Overview](#2.2.-component-overview)
- [3. Class Design](#3.-class-design)
  - [3.1. Class Diagram](#3.1.-class-diagram)
  - [3.2. Core Classes](#3.2.-core-classes)
    - [3.2.1. Logger Interface (`logger`)](#3.2.1.-logger-interface-%28%60logger%60%29)
    - [3.2.2. Logger Proxy (`logger_proxy`)](#3.2.2.-logger-proxy-%28%60logger_proxy%60%29)
    - [3.2.3. Real Logger (`real_logger`)](#3.2.3.-real-logger-%28%60real_logger%60%29)
- [4. Appender Design](#4.-appender-design)
  - [4.1. Appender Class Diagram](#4.1.-appender-class-diagram)
  - [4.2. Appender Details](#4.2.-appender-details)
    - [4.2.1. Console Appender](#4.2.1.-console-appender)
    - [4.2.2. File Appender](#4.2.2.-file-appender)
    - [4.2.3. Socket Appender](#4.2.3.-socket-appender)
- [5. Configuration System](#5.-configuration-system)
  - [5.1. Configuration Class Diagram](#5.1.-configuration-class-diagram)
  - [5.2. Configuration JSON Schema](#5.2.-configuration-json-schema)
  - [5.3. Appender Reference](#5.3.-appender-reference)
- [6. Pattern Engine](#6.-pattern-engine)
  - [6.1. Instance-Based Design](#6.1.-instance-based-design)
  - [6.2. Supported Placeholders](#6.2.-supported-placeholders)
- [7. Logger Manager](#7.-logger-manager)
  - [7.1. Singleton Pattern](#7.1.-singleton-pattern)
  - [7.2. Key Responsibilities](#7.2.-key-responsibilities)
- [8. Hot Configuration Reload](#8.-hot-configuration-reload)
  - [8.1. Key Mechanism: Proxy Pattern](#8.1.-key-mechanism%3A-proxy-pattern)
  - [8.2. Hot Reload Process](#8.2.-hot-reload-process)
  - [8.3. Object Lifetime Safety](#8.3.-object-lifetime-safety)
  - [8.4. Pattern Reload Safety](#8.4.-pattern-reload-safety)
  - [8.5. Implementation Details](#8.5.-implementation-details)
- [9. Thread Safety](#9.-thread-safety)
  - [9.1. Synchronization Strategy](#9.1.-synchronization-strategy)
  - [9.2. Locking Strategy](#9.2.-locking-strategy)
- [10. Data Flow](#10.-data-flow)
  - [10.1. Logging Data Flow](#10.1.-logging-data-flow)
- [11. Build System](#11.-build-system)
  - [11.1. CMake Structure](#11.1.-cmake-structure)
  - [11.2. Dependencies](#11.2.-dependencies)
- [12. Usage Example](#12.-usage-example)
  - [12.1. Basic Usage](#12.1.-basic-usage)
  - [12.2. Class Usage](#12.2.-class-usage)
- [13. Extension Points](#13.-extension-points)
  - [13.1. Adding Custom Appenders](#13.1.-adding-custom-appenders)
  - [13.2. Custom Pattern Tokens](#13.2.-custom-pattern-tokens)
<!-- /TOC -->

## 1. Overview

**log4cpp** is a C++ logging library inspired by Apache log4j. It provides a flexible, thread-safe logging framework with JSON-based configuration, multiple output appenders, and hot configuration reload capability.

### 1.1. Key Features

- JSON-based configuration (no code modification required)
- Multiple appender types: Console, File, Socket (TCP/UDP)
- Singleton pattern for global access
- Thread-safe implementation
- Hot configuration reload without process restart (Linux only)
- Pattern-based log formatting

---

## 2. Architecture

### 2.1. System Architecture Diagram

```mermaid
graph LR
    C[logger_manager] --> D[logger_proxy]
    D --> E[real_logger]
    E --> G[console_appender]
    E --> H[file_appender]
    E --> I[socket_appender]
    J[JSON] --> C
    K[supervisor] -.->|SIGHUP| C
```

### 2.2. Component Overview

| Component | Responsibility |
|-----------|----------------|
| `logger_manager` | Singleton managing all loggers, configuration loading |
| `logger_proxy` | Proxy pattern for hot-reload support |
| `real_logger` | Concrete logging implementation |
| `log_pattern` | Format log messages with patterns (per-logger instance) |
| `console_appender` | Output to stdout/stderr |
| `file_appender` | Output to file |
| `socket_appender` | Output to remote log server |

---

## 3. Class Design

### 3.1. Class Diagram

```mermaid
classDiagram
    class logger {
        <<abstract>>
        +get_name() string
        +set_name(name)
        +get_level() log_level
        +set_level(level)
        +log(level, fmt, args)
        +fatal(fmt, ...)
        +error(fmt, ...)
        +warn(fmt, ...)
        +info(fmt, ...)
        +debug(fmt, ...)
        +trace(fmt, ...)
    }

    class logger_proxy {
        -mtx: shared_mutex
        -target_: shared_ptr~logger~
        +get_target() shared_ptr~logger~
        +set_target(target)
        +log(level, fmt, args) override
        +info(fmt, ...) override
    }

    class real_logger {
        -name_: string
        -level_: log_level
        -appenders_mtx: shared_mutex
        -appenders: set~shared_ptr~log_appender~~
        -pattern_: log_pattern
        +log(level, fmt, args) override
        +add_appender(appender)
    }

    class log_appender {
        <<interface>>
        +log(msg, msg_len) = 0
    }

    class console_appender {
        -file_no: int
        -lock: log_lock
        +log(msg, msg_len) override
    }

    class file_appender {
        -fd: int
        -lock: log_lock
        +log(msg, msg_len) override
    }

    class socket_appender {
        -host: string
        -port: unsigned short
        -proto: protocol
        -sock_fd: socket_fd
        -reconnect_thread: thread
        +log(msg, msg_len) override
    }

    logger <|-- logger_proxy : Implementation
    logger <|-- real_logger : Implementation
    logger_proxy o-- logger : Delegates
    log_appender <|-- console_appender : Implementation
    log_appender <|-- file_appender : Implementation
    log_appender <|-- socket_appender : Implementation

    real_logger "1" *-- "n" log_appender : Composition
    real_logger "1" *-- "1" log_pattern : Composition
```

### 3.2. Core Classes

#### 3.2.1. Logger Interface (`logger`)

```cpp
// filepath: include/log4cpp/log4cpp.hpp
class logger {
public:
    virtual ~logger() = default;

    virtual std::string get_name() const = 0;
    virtual void set_name(const std::string &name) = 0;

    virtual log_level get_level() const = 0;
    virtual void set_level(log_level level) = 0;

    virtual void log(log_level _level, const char *fmt, va_list args) const = 0;

    // Convenience methods
    virtual void fatal(const char *fmt, ...) const = 0;
    virtual void error(const char *fmt, ...) const = 0;
    virtual void warn(const char *fmt, ...) const = 0;
    virtual void info(const char *fmt, ...) const = 0;
    virtual void debug(const char *fmt, ...) const = 0;
    virtual void trace(const char *fmt, ...) const = 0;
};
```

#### 3.2.2. Logger Proxy (`logger_proxy`)

The `logger_proxy` implements the **Proxy Design Pattern** to support hot configuration reload:

```cpp
// filepath: include/log4cpp/logger.hpp
class logger_proxy : public logger {
private:
    mutable std::shared_mutex mtx;
    std::shared_ptr<logger> target_;

public:
    explicit logger_proxy(std::shared_ptr<logger> target_logger);

    std::shared_ptr<logger> get_target();
    void set_target(std::shared_ptr<logger> target);  // Atomic swap
};
```

#### 3.2.3. Real Logger (`real_logger`)

Each `real_logger` holds its own `log_pattern` instance, eliminating global state and race conditions during hot-reload:

```cpp
// filepath: src/include/logger/real_logger.hpp
class real_logger : public logger {
private:
    std::string name_;
    log_level level_;
    mutable std::shared_mutex appenders_mtx;
    std::set<std::shared_ptr<appender::log_appender>> appenders;
    pattern::log_pattern pattern_;

public:
    void add_appender(const std::shared_ptr<appender::log_appender> &appender);
    void log(log_level _level, const char *fmt, va_list args) const override;
};
```

---

## 4. Appender Design

### 4.1. Appender Class Diagram

```mermaid
classDiagram
    class log_appender {
        <<interface>>
        +log(msg, msg_len) = 0
    }

    class console_appender {
        -file_no: int
        -lock: log_lock
        +log(msg, msg_len)
    }

    class file_appender {
        -fd: int
        -lock: log_lock
        +log(msg, msg_len)
        +~file_appender()
    }

    class socket_appender {
        -host: string
        -port: unsigned short
        -proto: protocol
        -sock_fd: socket_fd
        -reconnect_thread: thread
        -reconnect_mutex: mutex
        -reconnect_cv: condition_variable
        -stop_reconnect: atomic~bool~
        +log(msg, msg_len)
        +~socket_appender()
    }

    log_appender <|.. console_appender
    log_appender <|.. file_appender
    log_appender <|.. socket_appender
```

### 4.2. Appender Details

#### 4.2.1. Console Appender

Outputs log messages to stdout or stderr:

```cpp
// filepath: src/include/appender/console_appender.hpp
class console_appender : public log_appender {
private:
    int file_no = -1;
    common::log_lock lock;

public:
    explicit console_appender(const config::console_appender &cfg);
    void log(const char *msg, size_t msg_len) override;
};
```

#### 4.2.2. File Appender

Writes log messages to a specified file:

```cpp
// filepath: src/include/appender/file_appender.hpp
class file_appender : public log_appender {
private:
    int fd{-1};
    common::log_lock lock;

public:
    explicit file_appender(const config::file_appender &cfg);
    void log(const char *msg, size_t msg_len) override;
    ~file_appender() override;
};
```

#### 4.2.3. Socket Appender

Sends log messages to a remote log server via TCP or UDP:

```cpp
// filepath: src/include/appender/socket_appender.hpp
class socket_appender : public log_appender {
private:
    std::string host;
    unsigned short port{0};
    config::socket_appender::protocol proto;
    common::prefer_stack ip_stack;

    std::shared_mutex connection_rw_lock;
    common::socket_fd sock_fd;
    connection_fsm_state connection_state;

    std::mutex reconnect_mutex;
    std::condition_variable reconnect_cv;
    std::atomic<bool> stop_reconnect{false};
    std::thread reconnect_thread;

    // Exponential backoff: 1s to 24h
    static constexpr auto RECONNECT_INITIAL_DELAY = 1s;
    static constexpr auto RECONNECT_MAX_DELAY = 24h;
};
```

---

## 5. Configuration System

### 5.1. Configuration Class Diagram

```mermaid
classDiagram
    class log4cpp {
        -log_pattern: optional~string~
        -appenders: log_appender
        -loggers: unordered_map~string, logger~
    }

    class log_appender {
        -console: optional~console_appender~
        -file: optional~file_appender~
        -socket: optional~socket_appender~
    }

    class logger {
        -name: string
        -level: optional~log_level~
        -appender: unsigned char
    }

    class console_appender {
        -out_stream: string
    }

    class file_appender {
        -file_path: string
    }

    class socket_appender {
        -host: string
        -port: unsigned short
        -proto: protocol
        -prefer: prefer_stack
    }

    log4cpp --> log_appender
    log4cpp --> logger
    log_appender --> console_appender
    log_appender --> file_appender
    log_appender --> socket_appender
```

### 5.2. Configuration JSON Schema

```json
{
  "log-pattern": "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${msg}",
  "appenders": {
    "console": {
      "out-stream": "stdout"
    },
    "file": {
      "file-path": "/var/log/myapp.log"
    },
    "socket": {
      "host": "log-server.example.com",
      "port": 9999,
      "protocol": "TCP",
      "prefer-stack": "IPv4"
    }
  },
  "loggers": [
    {
      "name": "root",
      "level": "INFO",
      "appenders": ["console", "file", "socket"]
    },
    {
      "name": "myapp",
      "level": "DEBUG",
      "appenders": ["console", "file"]
    }
  ]
}
```

### 5.3. Appender Reference

Each logger declares which appenders it uses via the `"appenders"` string array. Only appenders defined in the top-level `"appenders"` object may be referenced. Valid names are `console`, `file`, and `socket`.

---

## 6. Pattern Engine

### 6.1. Instance-Based Design

The `log_pattern` class is an **instance-based** formatter. Each `real_logger` owns its own `log_pattern` instance, which is constructed from the logger's configured pattern string. This design eliminates global static state and removes the need for locks around pattern formatting, making hot-reload completely race-free at the pattern level.

```cpp
// filepath: src/include/pattern/log_pattern.hpp
class log_pattern {
public:
    explicit log_pattern(const std::string &pattern = DEFAULT_LOG_PATTERN);

    void set_pattern(const std::string &pattern);

    size_t format(char *__restrict buf, size_t buf_len, const char *name,
                  log_level level, const char *fmt, va_list args) const;
    size_t format(char *__restrict buf, size_t buf_len, const char *name,
                  log_level level, const char *fmt, ...) const;

private:
    std::string _pattern;
    void format_with_pattern(char *buf, size_t len, const char *name,
                             log_level level, const char *msg) const;
};
```

During hot-reload, the `logger_manager` creates a **new** `real_logger` with a **new** `log_pattern` instance. The old `real_logger` (and its pattern) remain valid until all in-flight logging calls complete, thanks to `shared_ptr` reference counting.

### 6.2. Supported Placeholders

| Token | Description | Example |
|-------|-------------|---------|
| `${yyyy}` | 4-digit year | 2026 |
| `${MM}` | 2-digit month | 04 |
| `${dd}` | 2-digit day | 24 |
| `${HH}` | 2-digit hour (24h) | 14 |
| `${mm}` | 2-digit minute | 30 |
| `${ss}` | 2-digit second | 45 |
| `${8TN}` | Thread name (padded to 8) | main    |
| `${L}` | Log level | INFO |
| `${msg}` | User message | Operation completed |

---

## 7. Logger Manager

### 7.1. Singleton Pattern

```mermaid
classDiagram
    class logger_manager {
        -static init_flag: once_flag
        -static instance: logger_manager
        -loggers_mtx: shared_mutex
        -loggers: unordered_map~string, shared_ptr~logger_proxy~~
        -config: config::log4cpp
        -config_file_path: string
        -evt_loop_thread: thread
    }

    class supervisor {
        -static get_logger_manager()$ logger_manager&
        -static enable_config_hot_loading() bool
        -static serialize() string
    }

    supervisor --> logger_manager : "static instance"
```

### 7.2. Key Responsibilities

1. **Singleton Management**: Ensure only one instance exists via `std::once_flag`
2. **Logger Creation**: Create and manage loggers by name via `get_logger()`
3. **Configuration Loading**: Parse JSON configuration files via `load_config()`
4. **Hot Reload**: Run event loop thread to receive SIGHUP signals (Linux)
5. **Appender Management**: Create and manage console, file, socket appenders
6. **Logger Lifecycle**: Use custom `logger_deleter` to auto-release loggers on destruction

---

## 8. Hot Configuration Reload

### 8.1. Key Mechanism: Proxy Pattern

The core of hot configuration reload is the **Proxy Design Pattern**. Clients hold a `shared_ptr<logger_proxy>` which forwards all logging calls to an internal `target_` pointer. When configuration changes:

```mermaid
sequenceDiagram
    participant Client
    participant Proxy as logger_proxy
    participant Old as real_logger (old)
    participant New as real_logger (new)
    participant Manager as logger_manager

    Note over Client: Client holds shared_ptr to logger_proxy
    Client->>Proxy: logger->info("msg")
    Proxy->>Old: forward to target_
    Old-->>Proxy: output log

    Note over Manager: Config file changed
    Manager->>Manager: load new config
    Manager->>Manager: create new real_logger

    loop For each logger
        Manager->>Proxy: set_target(new_real_logger)
    end

    Note over Proxy: target_ swapped atomically

    Client->>Proxy: logger->info("msg")
    Proxy->>New: forward to new target_
    New-->>Proxy: output log
```

### 8.2. Hot Reload Process

1. **Client holds proxy**: Users get `shared_ptr<logger_proxy>` which never changes
2. **Signal received**: SIGHUP triggers `notify_config_hot_reload()`
3. **Create new loggers**: Manager creates new `real_logger` instances from updated config, each with its own `log_pattern`
4. **Atomic swap**: `logger_proxy::set_target()` replaces `target_` pointer
5. **Transparent to client**: No code changes needed, proxy forwards to new implementation

### 8.3. Object Lifetime Safety

The key challenge: **how to ensure in-use objects don't become invalid during reload?**

```mermaid
sequenceDiagram
    participant Client
    participant Proxy as logger_proxy
    participant Old as real_logger (old)
    participant New as real_logger (new)
    participant Manager as logger_manager

    Note over Client: 1. Client holds shared_ptr to proxy
    Note over Manager: 2. Manager holds weak_ptr to proxy

    Client->>Proxy: logger->info("msg")
    Proxy->>Old: forward (holds shared_ptr)
    Old-->>Proxy: output log

    Note over Manager: 3. Config changed, create new
    Manager->>Manager: create new real_logger

    Note over Manager: 4. Swap - old still alive!
    Manager->>Proxy: set_target(new)

    Note over Old: Still referenced by in-flight calls
    Note over New: Now receiving new calls

    Client->>Proxy: logger->info("msg")
    Proxy->>New: forward to new target_
    New-->>Proxy: output log

    Note over Old: Eventually released when<br/>no more references exist
```

**Lifetime management strategy:**

- **Client → Proxy**: Client holds `shared_ptr<logger_proxy>` (never changes)
- **Proxy → Real Logger**: Proxy holds `shared_ptr<logger>` to `target_`
- **Manager → Loggers**: Manager holds `weak_ptr<logger_proxy>` to track loggers
- **Old logger kept alive**: Old `real_logger` remains valid until all in-flight calls complete

```cpp
// When swapping, old logger is NOT deleted immediately
// It remains alive because proxy still holds shared_ptr to it
void logger_proxy::set_target(std::shared_ptr<logger> target) {
    std::unique_lock lock(mtx);
    // Old target_ automatically kept alive by the shared_ptr
    // Only released when no more callers hold reference to proxy
    target_ = target;
}
```

**Why this works:**
- `shared_ptr` reference counting keeps old object alive
- Old logger is released only when **all** proxies have been updated
- In-flight logging calls complete with old configuration
- New logging calls use new configuration

```cpp
// Hot reload atomic swap
void logger_proxy::set_target(std::shared_ptr<logger> target) {
    std::unique_lock lock(mtx);
    target_ = target;  // Atomic at shared_ptr level
}
```

### 8.4. Pattern Reload Safety

Because each `real_logger` owns its own `log_pattern` instance, pattern changes during hot-reload are inherently safe:

- Old `real_logger` uses its own `log_pattern` (frozen at creation time)
- New `real_logger` uses a new `log_pattern` instance (from new config)
- No global mutex or shared state on the pattern formatter
- Formatting is `const`-qualified and lock-free

### 8.5. Implementation Details

- **Signal Handler**: Register SIGHUP handler via `supervisor::enable_config_hot_loading()`
- **Event Loop**: Background thread using `eventfd` to receive reload signals
- **Thread Safety**: `std::shared_mutex` protects the `target_` pointer
  - Logging operations use shared lock (multiple readers)
  - `set_target()` uses unique lock (single writer)

---

## 9. Thread Safety

### 9.1. Synchronization Strategy

```mermaid
graph TB
    subgraph "Read-Heavy Workloads"
        A[Logger Operations log debug info]
    end

    subgraph "Write-Heavy Workloads"
        B[Configuration add_appender]
        C[Hot Reload set_target]
    end

    D[shared_mutex] --> A
    D[shared_mutex] --> B
    D[shared_mutex] --> C

    style A fill:#90EE90
    style B fill:#FFB6C1
    style C fill:#FFB6C1
```

### 9.2. Locking Strategy

| Component | Lock Type | Purpose |
|-----------|-----------|---------|
| `logger_proxy::mtx` | `shared_mutex` | Protect `target_` pointer |
| `real_logger::appenders_mtx` | `shared_mutex` | Protect appender set |
| `socket_appender::connection_rw_lock` | `shared_mutex` | Protect socket connection |
| `console_appender::lock` | `log_lock` | Platform-specific file locking |
| `file_appender::lock` | `log_lock` | Platform-specific file locking |

---

## 10. Data Flow

### 10.1. Logging Data Flow

```mermaid
flowchart TD
    A[User Code logger.info] --> B[logger_proxy]
    B --> C{Hot Reload In Progress?}
    C -->|No| D[real_logger]
    C -->|Yes| E[New real_logger]
    E --> D
    D --> F[log_pattern format message]
    F --> G[appenders set]
    G --> H[console_appender]
    G --> I[file_appender]
    G --> J[socket_appender]
    H --> K[stdout stderr]
    I --> L[File]
    J --> M[Remote Server]
```

---

## 11. Build System

### 11.1. CMake Structure

```mermaid
graph TD
    subgraph "Project Root"
        A[CMakeLists.txt]
    end

    subgraph "Source"
        B[src/CMakeLists.txt]
    end

    subgraph "Headers"
        C[include/log4cpp/]
        D[src/include/]
    end

    subgraph "Tests"
        E[test/CMakeLists.txt]
    end

    A --> B
    A --> E
    B --> C
    B --> D
```

### 11.2. Dependencies

- **Required**: CMake 3.10+, C++17 compiler
- **Platform**: Linux (hot reload), Windows/macOS (basic)

---

## 12. Usage Example

### 12.1. Basic Usage

```cpp
#include <log4cpp/log4cpp.hpp>

int main() {
    // Load configuration
    log4cpp::supervisor::get_logger_manager().load_config("./log4cpp.json");

    // Get logger
    auto logger = log4cpp::supervisor::get_logger_manager().get_logger("myapp");

    // Log messages
    logger->info("Application started");
    logger->debug("Processing request: %s", request_id);
    logger->error("Failed to connect to database");

    return 0;
}
```

### 12.2. Class Usage

```cpp
class MyClass {
private:
    std::shared_ptr<log4cpp::logger> logger_;

public:
    MyClass() {
        logger_ = log4cpp::supervisor::get_logger_manager().get_logger("MyClass");
    }

    void process() {
        logger_->info("Processing in MyClass");
    }
};
```

---

## 13. Extension Points

### 13.1. Adding Custom Appenders

```cpp
class my_appender : public log4cpp::appender::log_appender {
public:
    explicit my_appender(const config::my_appender &cfg) : config_(cfg) {}

    void log(const char *msg, size_t msg_len) override {
        // Custom implementation
    }

private:
    config::my_appender config_;
};
```

### 13.2. Custom Pattern Tokens

Extend the `format_with_pattern()` method inside `log_pattern` to support additional tokens. Because `log_pattern` is an instance class, you can also create a derived formatter and inject it via a custom `logger_builder` if needed:

```cpp
namespace log4cpp::pattern {
    // Add new token handling in format_with_pattern()
}
```
