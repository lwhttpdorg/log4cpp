#pragma once

#ifdef _MSC_VER
#include <synchapi.h>
#else
#include <pthread.h>
#endif

namespace log4cpp {
	class log_lock final {
	public:
		log_lock() {
#ifdef _MSC_VER
			(void)InitializeCriticalSectionAndSpinCount(&_m_lock, 0x00000400);
#else
			pthread_spin_init(&_m_lock, PTHREAD_PROCESS_PRIVATE);
#endif
		}

		~log_lock() {
#ifdef _MSC_VER
			DeleteCriticalSection(&_m_lock);
#else
			pthread_spin_destroy(&_m_lock);
#endif
		}

		void lock() {
#ifdef _MSC_VER
			EnterCriticalSection(&_m_lock);
#else
			pthread_spin_lock(&_m_lock);
#endif
		}

		void unlock() {
#ifdef _MSC_VER
			LeaveCriticalSection(&_m_lock);
#else
			pthread_spin_unlock(&_m_lock);
#endif
		}

	private:
#ifdef _MSC_VER
		CRITICAL_SECTION _m_lock{};
#else
		pthread_spinlock_t _m_lock{};
#endif
	};
}
