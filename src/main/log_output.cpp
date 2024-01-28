#include <chrono>
#include <thread>
#include <csignal>

#if defined(__WIN32)

#include <processthreadsapi.h>

#endif

#include "log_output.h"
#include "log_utils.h"

using namespace log4cpp;

size_t log_output::build_prefix(log_level level, char *buf, size_t len)
{
	size_t used_len = 0;
	std::chrono::system_clock::time_point clock_now = std::chrono::system_clock::now();
	std::time_t tm_now = std::chrono::system_clock::to_time_t(clock_now);
	tm *local = localtime(&tm_now);
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "%04d-%02d-%02d %02d:%02d:%02d ",
	                            1900 + local->tm_year, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min,
	                            local->tm_sec);
	char thread_name[16];
	thread_name[0] = '\0';
#ifdef _GNU_SOURCE
	pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
#elif defined(__linux__)
	prctl(PR_GET_NAME, (unsigned long)thread_name);
#endif
	if (thread_name[0] == '\0')
	{
#if defined(_MSC_VER) || defined(__WIN32)
		unsigned long tid = GetCurrentThreadId();
#endif
#ifdef __linux__
		unsigned long tid = gettid();
#endif
		log4c_scnprintf(thread_name, sizeof(thread_name), "%u", tid);
	}
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%16s]: ", thread_name);
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%-5s] -- ", log4cpp::to_string(level).c_str());
	return used_len;
}

lock_singleton::inner_garbo lock_singleton::garbo;
std::mutex lock_singleton::mutex_lock;
lock_singleton *lock_singleton::instance = nullptr;
