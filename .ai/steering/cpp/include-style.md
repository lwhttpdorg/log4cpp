---
description: C++ include ordering rules. Read this only when editing C++ source or header files.
applies_to:
  - "**/*.hpp"
  - "**/*.cpp"
---

# C++ Include Style

Follow these rules only when you add, remove, or reorder C++ include directives.

Do not apply this steering file to Markdown, build scripts, packaging files, or other non-C++ files.

## 1. Required Order

Group include directives in this exact order:

1. C and C++ standard library headers
2. System and platform library headers
3. Third-party library headers
4. Project headers

Put one blank line between adjacent groups. Do not add blank lines inside a group unless platform guards require it.

## 2. Bracket Style

Use angle brackets for standard, system, platform, and third-party headers.

```cpp
#include <string>       // for std::string
#include <sys/socket.h> // for socket
```

Use double quotes for every project header.

```cpp
#include "common/log_utils.hpp" // for log4c_scnprintf
#include "log4cpp/log4cpp.hpp"  // for log_level
```

Never include project headers with angle brackets, even if the build system accepts it.

## 3. Include Comments

Every include directive must have a short `// for ...` comment.

The comment should name the main symbol, function, type, or feature that requires the header.

```cpp
#include <mutex> // for std::mutex
#include <unistd.h> // for read, write, close
#include "common/log_net.hpp" // for net_addr
```

Keep comments concise and specific. Do not write broad comments such as `// for utilities`.

## 4. Platform Guards

Keep platform-specific headers in the correct group and guard them with the relevant platform macro.

Prefer explicit platform guards when the API is platform-specific:

```cpp
#ifdef _WIN32
#include <windows.h> // for Windows API
#endif

#ifdef __linux__
#include <sys/eventfd.h> // for eventfd
#endif
```

Use `#ifndef _WIN32` for POSIX or BSD APIs that are shared by non-Windows platforms:

```cpp
#ifndef _WIN32
#include <unistd.h> // for read, write, close
#endif
```

## 5. Example

```cpp
#include <cstring>    // for std::strerror
#include <filesystem> // for std::filesystem
#include <mutex>      // for std::scoped_lock

#include <fcntl.h> // for open flags
#ifdef _WIN32
#include <io.h> // for _open, _close, _write
#endif
#ifndef _WIN32
#include <unistd.h> // for close, write
#endif

#include "appender/file_appender.hpp" // for file_appender
```
