#include <cstring>    // for strerror
#include <filesystem> // for std::filesystem
#include <mutex>      // for std::scoped_lock
#include <stdexcept>  // for std::runtime_error

#include <fcntl.h> // for open flags
#ifdef _MSC_VER
#include <windows.h> // for Windows file constants
#endif
#ifdef _WIN32
#include <direct.h> // for _mkdir
#include <io.h>     // for _open, _close, _write
#endif
#ifdef __MINGW32__
#include <sys/stat.h> // for mode constants
#endif
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/stat.h> // for mkdir, mode_t
#include <unistd.h>   // for close, write
#endif

#include "appender/file_appender.hpp" // for file_appender

namespace log4cpp::appender {
    file_appender::file_appender(const config::file_appender &cfg) {
        if (const auto pos = cfg.file_path.find_last_of('/'); pos != std::string::npos) {
            std::string path = cfg.file_path.substr(0, pos);
            if (!std::filesystem::exists(path)) {
#ifdef _WIN32
                if (_mkdir(path.c_str()) == -1) {
                    std::string what("Can not create log directory '");
                    what.append(path);
                    what.append("': ");
                    what.append(strerror(errno));
                    what.append("(" + std::to_string(errno) + ")");
                    throw std::runtime_error(what);
                }
#endif
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
                if (mkdir(path.c_str(), 0755) == -1) {
                    std::string what("Can not create log directory '");
                    what.append(path);
                    what.append("': ");
                    what.append(strerror(errno));
                    what.append("(" + std::to_string(errno) + ")");
                    throw std::runtime_error(what);
                }
#endif
            }
        }
        int openFlags = O_RDWR | O_CREAT | O_APPEND;
#ifdef _WIN32
        int mode = _S_IREAD | _S_IWRITE;
#endif

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#ifdef O_CLOEXEC
        openFlags |= O_CLOEXEC;
#endif
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
#ifdef _MSC_VER
        this->fd = _open(cfg.file_path.c_str(), openFlags, mode);
#else
        this->fd = open(cfg.file_path.c_str(), openFlags, mode);
#endif
        if (this->fd == -1) {
            std::string what("Can not open log file '");
            what.append(cfg.file_path);
            what.append("': ");
            what.append(strerror(errno));
            what.append("(" + std::to_string(errno) + ")");
            throw std::runtime_error(what);
        }
    }

    file_appender::~file_appender() {
        if (this->fd != -1) {
#ifdef _MSC_VER
            _close(this->fd);
#else
            close(this->fd);
#endif
        }
    }

    void file_appender::log(const char *msg, size_t msg_len) {
        std::scoped_lock fd_lock(this->lock);
#ifdef _MSC_VER
        (void)_write(this->fd, msg, static_cast<unsigned int>(msg_len));
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        (void)write(this->fd, msg, msg_len);
#pragma GCC diagnostic pop
#endif
    }
} // namespace log4cpp::appender
