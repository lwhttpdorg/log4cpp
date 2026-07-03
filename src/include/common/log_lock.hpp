#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#include <mutex> // for std::mutex
#endif

#ifdef _MSC_VER
#include <synchapi.h> // for CRITICAL_SECTION
#endif
#ifdef __linux__
#include <pthread.h> // for pthread_spinlock_t
#endif

namespace log4cpp::common {
    class log_lock final {
    public:
        log_lock() {
#ifdef _MSC_VER
            (void)InitializeCriticalSectionAndSpinCount(&_m_lock, 0x00000400);
#endif
#ifdef __linux__
            pthread_spin_init(&_m_lock, PTHREAD_PROCESS_PRIVATE);
#endif
        }

        ~log_lock() {
#ifdef _MSC_VER
            DeleteCriticalSection(&_m_lock);
#endif
#ifdef __linux__
            pthread_spin_destroy(&_m_lock);
#endif
        }

        log_lock(const log_lock &other) = delete;

        log_lock(log_lock &&other) = delete;

        log_lock &operator=(const log_lock &other) = delete;

        log_lock &operator=(log_lock &&other) = delete;

        void lock() {
#ifdef _MSC_VER
            EnterCriticalSection(&_m_lock);
#endif
#if defined(__APPLE__) && defined(__MACH__)
            _m_lock.lock();
#endif
#ifdef __linux__
            pthread_spin_lock(&_m_lock);
#endif
        }

        void unlock() {
#ifdef _MSC_VER
            LeaveCriticalSection(&_m_lock);
#endif
#if defined(__APPLE__) && defined(__MACH__)
            _m_lock.unlock();
#endif
#ifdef __linux__
            pthread_spin_unlock(&_m_lock);
#endif
        }

    private:
#ifdef _MSC_VER
        CRITICAL_SECTION _m_lock{};
#endif
#if defined(__APPLE__) && defined(__MACH__)
        std::mutex _m_lock{};
#endif
#ifdef __linux__
        pthread_spinlock_t _m_lock{};
#endif
    };
} // namespace log4cpp::common
