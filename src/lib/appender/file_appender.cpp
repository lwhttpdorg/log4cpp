#include <fcntl.h>
#include <stdexcept>

#ifdef _MSC_VER
#include <windows.h>
#endif

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

#ifdef __MINGW32__

#include <sys/stat.h>

#endif

#if defined(__linux__)

#include <sys/stat.h>
#include <unistd.h>

#endif

#include <mutex>

#include "appender/file_appender.hpp"

namespace log4cpp::appender {
    file_appender::file_appender(const config::file_appender &cfg) {
        if (const auto pos = cfg.file_path.find_last_of('/'); pos != std::string::npos) {
            std::string path = cfg.file_path.substr(0, pos);
            if (!std::filesystem::exists(path)) {
#ifdef _WIN32
                (void)_mkdir(path.c_str());
#endif
#if defined(__linux__)
                mkdir(path.c_str(), 0755);
#endif
            }
        }
        int openFlags = O_RDWR | O_CREAT | O_APPEND;
#ifdef _WIN32
        int mode = _S_IREAD | _S_IWRITE;
#endif

#ifdef __linux__
        openFlags |= O_CLOEXEC;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
#ifdef _MSC_VER
        this->fd = _open(cfg.file_path.c_str(), openFlags, mode);
#else
        this->fd = open(cfg.file_path.c_str(), openFlags, mode);
#endif
        if (this->fd == -1) {
            std::string what("Can not open log file: ");
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
        std::lock_guard lock_guard(this->lock);
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
