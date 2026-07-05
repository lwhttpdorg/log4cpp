#include <algorithm>  // for std::sort
#include <chrono>     // for std::chrono
#include <cstring>    // for strerror
#include <ctime>      // for localtime_r, strftime
#include <filesystem> // for std::filesystem
#include <mutex>      // for std::scoped_lock
#include <stdexcept>  // for std::runtime_error
#include <vector>     // for std::vector

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

#include "appender/file_appender.hpp"               // for file_appender
#include "appender/rolling_file_appender_rules.hpp" // for rolling_file_appender_rules

namespace log4cpp::appender {
    static std::tm system_localtime_now() {
        const std::time_t now = std::time(nullptr);
        std::tm now_tm{};
#ifdef _WIN32
        localtime_s(&now_tm, &now);
#else
        localtime_r(&now, &now_tm);
#endif
        return now_tm;
    }

    static std::filesystem::path next_indexed_archive_path(const std::filesystem::path &parent,
                                                           const std::string &archive_prefix) {
        for (uint32_t idx = 1;; ++idx) {
            std::filesystem::path candidate = parent / (archive_prefix + "." + std::to_string(idx));
            if (!std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }

    file_appender::file_appender(const config::file_appender &cfg) : file_path(cfg.file_path), rolling(cfg.rolling) {
        ensure_parent_dir();
        roll_on_start();
        open_file();
        this->opened_time_bucket = current_time_bucket();
    }

    void file_appender::ensure_parent_dir() const {
        if (const auto pos = this->file_path.find_last_of('/'); pos != std::string::npos) {
            std::string path = this->file_path.substr(0, pos);
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
    }

    void file_appender::open_file() {
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
        this->fd = _open(this->file_path.c_str(), openFlags, mode);
#else
        this->fd = open(this->file_path.c_str(), openFlags, mode);
#endif
        if (this->fd == -1) {
            std::string what("Can not open log file '");
            what.append(this->file_path);
            what.append("': ");
            what.append(strerror(errno));
            what.append("(" + std::to_string(errno) + ")");
            throw std::runtime_error(what);
        }

        if (std::filesystem::exists(this->file_path)) {
            this->current_size = std::filesystem::file_size(this->file_path);
        }
        else {
            this->current_size = 0;
        }
    }

    void file_appender::close_file() {
        if (this->fd != -1) {
#ifdef _MSC_VER
            _close(this->fd);
#else
            close(this->fd);
#endif
            this->fd = -1;
        }
    }

    file_appender::~file_appender() {
        close_file();
    }

    void file_appender::roll_on_start() {
        if (!this->rolling || this->rolling->policy != config::rolling_policy_type::ON_START) {
            return;
        }
        if (!std::filesystem::exists(this->file_path) || std::filesystem::file_size(this->file_path) == 0) {
            return;
        }
        std::filesystem::rename(this->file_path, archive_path());
        cleanup_archives();
    }

    void file_appender::roll() {
        close_file();
        if (std::filesystem::exists(this->file_path) && std::filesystem::file_size(this->file_path) > 0) {
            std::filesystem::rename(this->file_path, archive_path());
        }
        open_file();
        this->opened_time_bucket = current_time_bucket();
        cleanup_archives();
    }

    void file_appender::maybe_roll(size_t msg_len) {
        if (rolling_file_appender_rules::should_roll(this->rolling, this->current_size, msg_len,
                                                     this->opened_time_bucket, current_time_bucket())) {
            roll();
        }
    }

    std::string file_appender::archive_path() const {
        const std::filesystem::path active_path(this->file_path);
        const std::filesystem::path parent = active_path.parent_path();
        const std::string filename = active_path.filename().string();

        const std::string archive_prefix = rolling_file_appender_rules::archive_prefix(
            this->rolling, filename, this->opened_time_bucket, current_time_bucket());
        std::filesystem::path candidate = parent / archive_prefix;
        if (this->rolling && this->rolling->policy == config::rolling_policy_type::TIME
            && !std::filesystem::exists(candidate)) {
            return candidate.string();
        }

        return next_indexed_archive_path(parent, archive_prefix).string();
    }

    std::string file_appender::format_time(const char *fmt) const {
        const std::tm now_tm = system_localtime_now();
        char buf[32];
        if (std::strftime(buf, sizeof(buf), fmt, &now_tm) == 0) {
            return {};
        }
        return buf;
    }

    std::string file_appender::current_time_bucket() const {
        if (!rolling_file_appender_rules::uses_time_bucket(this->rolling)) {
            return {};
        }
        if (this->rolling->interval == config::rolling_time_interval::HOUR) {
            return format_time("%Y%m%d%H");
        }
        return format_time("%Y%m%d");
    }

    void file_appender::cleanup_archives() const {
        if (!this->rolling || (this->rolling->file_count == 0 && this->rolling->max_history.count() == 0)) {
            return;
        }

        const std::filesystem::path active_path(this->file_path);
        const std::filesystem::path parent =
            active_path.parent_path().empty() ? std::filesystem::path(".") : active_path.parent_path();
        const std::string archive_prefix = active_path.filename().string() + ".";

        std::vector<std::filesystem::directory_entry> archives;
        for (const auto &entry: std::filesystem::directory_iterator(parent)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string name = entry.path().filename().string();
            if (name.starts_with(archive_prefix)) {
                archives.emplace_back(entry);
            }
        }

        const auto now = std::filesystem::file_time_type::clock::now();
        if (this->rolling->max_history.count() > 0) {
            const auto max_history = this->rolling->max_history;
            archives.erase(std::remove_if(archives.begin(), archives.end(),
                                          [now, max_history](const std::filesystem::directory_entry &entry) {
                                              const auto age = now - entry.last_write_time();
                                              if (age > max_history) {
                                                  std::filesystem::remove(entry.path());
                                                  return true;
                                              }
                                              return false;
                                          }),
                           archives.end());
        }

        if (this->rolling->file_count > 0 && archives.size() > this->rolling->file_count) {
            std::sort(archives.begin(), archives.end(),
                      [](const std::filesystem::directory_entry &lhs, const std::filesystem::directory_entry &rhs) {
                          return lhs.last_write_time() > rhs.last_write_time();
                      });
            for (size_t idx = this->rolling->file_count; idx < archives.size(); ++idx) {
                std::filesystem::remove(archives[idx].path());
            }
        }
    }

    void file_appender::log(const char *msg, size_t msg_len) {
        std::scoped_lock fd_lock(this->lock);
        maybe_roll(msg_len);
#ifdef _MSC_VER
        (void)_write(this->fd, msg, static_cast<unsigned int>(msg_len));
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        (void)write(this->fd, msg, msg_len);
#pragma GCC diagnostic pop
#endif
        this->current_size += msg_len;
    }
} // namespace log4cpp::appender
