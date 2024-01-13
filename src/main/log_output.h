#pragma once

#include <cstddef>
#include <mutex>


#include "log4cpp.hpp"
#include "log_lock.h"

namespace log4cpp
{
    constexpr unsigned short LOG_LINE_MAX = 512;
    const std::string CONSOLE_OUTPUT_NAME = "consoleOutPut";
    const std::string FILE_OUTPUT_NAME = "fileOutPut";
    const std::string TCP_OUTPUT_NAME = "tcpOutPut";
    const std::string UDP_OUTPUT_NAME = "udpOutPut";

    class log_output
    {
    public:
        static size_t build_prefix(log_level level, char *__restrict buf, size_t len);

        virtual void log(log_level level, const char *__restrict fmt, va_list args) = 0;

        virtual void log(log_level level, const char *__restrict fmt, ...) = 0;

        virtual ~log_output() = default;
    };

    class lock_singleton
    {
    public:
        static lock_singleton *get_instance()
        {
            if (instance == nullptr)
            {
                mutex_lock.lock();
                if (instance == nullptr)
                {
                    instance = new lock_singleton();
                }
                mutex_lock.unlock();
            }
            return instance;
        }

        lock_singleton(const lock_singleton &obj) = delete;

        lock_singleton &operator=(const lock_singleton &) = delete;

        void lock()
        {
            _lock.lock();
        }

        void unlock()
        {
            _lock.unlock();
        }

    private:
        lock_singleton() = default;

    private:
        class inner_garbo
        {
        public:
            virtual ~inner_garbo()
            {
                if (lock_singleton::instance != nullptr)
                {
                    delete lock_singleton::instance;
                    lock_singleton::instance = nullptr;
                }
            }
        };

    private:
        log_lock _lock;
        static std::mutex mutex_lock;
        static lock_singleton *instance;
        static inner_garbo garbo;
    };
}
