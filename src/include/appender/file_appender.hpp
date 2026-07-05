#pragma once

#include <cstdint>  // for uint64_t
#include <optional> // for std::optional
#include <string>   // for std::string

#include "appender/log_appender.hpp" // for log_appender
#include "common/log_lock.hpp"       // for log_lock
#include "config/appender.hpp"       // for file_appender, rolling_policy

namespace log4cpp::appender {
    class file_appender: public log_appender {
    public:
        explicit file_appender(const config::file_appender &cfg);
        file_appender(const file_appender &other) = delete;

        file_appender(file_appender &&other) = delete;

        file_appender &operator=(const file_appender &other) = delete;

        file_appender &operator=(file_appender &&other) = delete;

        void log(const char *msg, size_t msg_len) override;

        ~file_appender() override;

    private:
        void ensure_parent_dir() const;
        void open_file();
        void close_file();
        void roll();
        void roll_on_start();
        void maybe_roll(size_t msg_len);
        void cleanup_archives() const;
        [[nodiscard]] std::string archive_path() const;
        [[nodiscard]] std::string current_time_bucket() const;
        [[nodiscard]] std::string format_time(const char *fmt) const;

        std::string file_path;
        std::optional<config::rolling_policy> rolling;
        uint64_t current_size{0};
        std::string opened_time_bucket;

        /* The fd of the log file */
        int fd{-1};
        common::log_lock lock;
    };
} // namespace log4cpp::appender
