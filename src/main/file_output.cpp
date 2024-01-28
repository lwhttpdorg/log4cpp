#ifdef _MSC_VER
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <fcntl.h>
#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <mutex>
#include <sys/stat.h>

#if defined(__WIN32)

#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif

#define STDOUT_FILENO _fileno(stdout)
#define F_OK 0

#endif

#if defined(_WIN32) || defined(_MSC_VER)

#include <direct.h>
#include <io.h>
#include <processthreadsapi.h>

#endif

#if defined(__linux__)

#include <unistd.h>

#endif

#include "log4cpp.hpp"
#include "file_output.h"
#include "main/log_utils.h"
#include "log4cpp_config.h"


using namespace log4cpp;

file_output::builder &file_output::builder::set_file(const std::string &file)
{
    if (this->instance == nullptr)
    {
        throw std::runtime_error("Call new_builder() first");
    }
    auto pos = file.find_last_of('/');
    if (pos != std::string::npos)
    {
        std::string path = file.substr(0, pos);
        if (0 != access(path.c_str(), F_OK))
        {
#if defined(_MSC_VER) || defined(__WIN32)
            (void)_mkdir(path.c_str());
#endif
#if defined(__linux__)
            mkdir(path.c_str(), 0755);
#endif
        }
    }
#if defined(_MSC_VER) || defined(__WIN32)
    int openFlags = O_RDWR|O_APPEND|O_CREAT;
    int mode = _S_IREAD|_S_IWRITE;

#endif

#ifdef __linux__
    int openFlags = O_RDWR | O_APPEND | O_CLOEXEC | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
    this->instance->fd = open(file.c_str(), openFlags, mode);
    if (this->instance->fd == -1)
    {
        std::string what("Can not open log file, ");
        what.append(strerror(errno));
        what.append("(" + std::to_string(errno) + ")");
        throw std::runtime_error(what);
    }
}

file_output *file_output::builder::build()
{
    return this->instance;
}

file_output::builder file_output::builder::new_builder()
{
    file_output::builder builder = file_output::builder{};
    builder.instance = new file_output();
    return builder;
}

file_output::builder &file_output::builder::set_async(bool async)
{
    return *this;
}

file_output::builder &file_output::builder::set_append(bool append)
{
    return *this;
}

file_output::file_output(file_output &&other) noexcept
{
    this->fd = other.fd;
    other.fd = -1;
}

file_output &file_output::operator=(file_output &&other) noexcept
{
    if (this != &other)
    {
        this->fd = other.fd;
        other.fd = -1;
    }
    return *this;
}

file_output::~file_output()
{
    if (this->fd != -1)
    {
        close(this->fd);
    }
}

void file_output::log(log_level level, const char *fmt, va_list args)
{
    char buffer[LOG_LINE_MAX];
    size_t used_len = 0, buf_len = sizeof(buffer);
    buffer[0] = '\0';
    used_len += build_prefix(level, buffer, buf_len);
    used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
    used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
    lock_singleton *lock = lock_singleton::get_instance();
    lock->lock();
    (void) write(this->fd, buffer, used_len);
    lock->unlock();
}

void file_output::log(log_level level, const char *fmt, ...)
{
    char buffer[LOG_LINE_MAX];
    size_t used_len = 0, buf_len = sizeof(buffer);
    buffer[0] = '\0';
    used_len += build_prefix(level, buffer, buf_len);
    va_list args;
    va_start(args, fmt);
    used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
    va_end(args);
    used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
    lock_singleton *lock = lock_singleton::get_instance();
    lock->lock();
    (void) write(this->fd, buffer, used_len);
    lock->unlock();
    lock->unlock();
}

file_output *file_output_config::instance = nullptr;
log_lock file_output_config::instance_lock;

file_output *file_output_config::get_instance(const file_output_config &self)
{
    if (instance == nullptr)
    {
        instance_lock.lock();
        if (instance == nullptr)
        {
            instance = file_output::builder::new_builder().set_file(self.file_path).build();
        }
        instance_lock.unlock();
    }
    return instance;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const file_output_config &obj)
{
    json = boost::json::object{{"filePath", obj.file_path},
                               {"async",    obj.async},
                               {"append",   obj.append}};
}

file_output_config log4cpp::tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json)
{
    file_output_config config;
    config.file_path = boost::json::value_to<std::string>(json.at("filePath"));
    config.async = boost::json::value_to<bool>(json.at("async"));
    config.append = boost::json::value_to<bool>(json.at("append"));
    return config;
}
