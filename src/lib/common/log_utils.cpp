#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

#ifdef _MSC_VER
#include <Windows.h>
#include <processthreadsapi.h>
#include <winerror.h>
#elif defined(__GNUC__)

#include <pthread.h>

#else
#include <sys/prctl.h>
#endif

#include "common/log_utils.hpp"

namespace log4cpp {
    /************************thread name*************************/
    unsigned long get_thread_name_id(char *thread_name, size_t len) {
        thread_name[0] = '\0';
#ifdef _MSC_VER
        (void)len;
        unsigned long tid = GetCurrentThreadId();
        PWSTR thread_name_ptr;
        HRESULT hresult = GetThreadDescription(GetCurrentThread(), &thread_name_ptr);
        if (SUCCEEDED(hresult)) {
            std::wstring w_name(thread_name_ptr);
            LocalFree(thread_name_ptr);
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, w_name.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string name(size_needed, '\0');
            WideCharToMultiByte(CP_UTF8, 0, w_name.c_str(), -1, &name[0], size_needed, nullptr, nullptr);
            common::log4c_scnprintf(thread_name, len, "%s", name.c_str());
        }
#endif

#ifdef __GNUC__
        unsigned long tid = pthread_self();
        pthread_getname_np(pthread_self(), thread_name, len);
#elif __linux__
        unsigned long tid = gettid();
        (void)len;
        prctl(PR_GET_NAME, reinterpret_cast<unsigned long>(thread_name));
#endif
        return tid;
    }

    void set_thread_name(const char *name) {
#ifdef _MSC_VER
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        std::wstring wchar_str(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, name, -1, &wchar_str[0], size_needed);
        SetThreadDescription(GetCurrentThread(), wchar_str.c_str());
#endif
#ifdef __GNUC__
        char buf[16]; // 15 chars + '\0'
        strncpy(buf, name, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        pthread_setname_np(pthread_self(), buf);
#elif __linux__
        char buf[16];
        strncpy(buf, name, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        prctl(PR_SET_NAME, buf);
#endif
    }
}

namespace log4cpp::common {
    size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
        int i = vsnprintf(buf, size, fmt, args);
        if (i > 0) {
            return i;
        }
        return 0;
    }

    size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int i = vsnprintf(buf, size, fmt, args);
        va_end(args);
        if (i > 0) {
            return i;
        }
        return 0;
    }

    void log4c_debug(FILE *__restrict stream, const char *__restrict fmt, ...) {
        const std::time_t now = std::time(nullptr);
        char timebuf[32];
#ifdef _WIN32
        ctime_s(timebuf, sizeof(timebuf), &now);
#else
        ctime_r(&now, timebuf);
#endif
        timebuf[strcspn(timebuf, "\n")] = '\0';
        char buffer[192];
        size_t len = log4c_scnprintf(buffer, sizeof(buffer), "[DEBUG] %s: ", timebuf);
        va_list args;
        va_start(args, fmt);
        log4c_vscnprintf(buffer + len, sizeof(buffer) - len, fmt, args);
        va_end(args);
        fprintf(stream, "%s", buffer);
        fflush(stream);
    }

    size_t log4c_replace(char *original, size_t length, const char *target, const char *replace) {
        size_t target_len = strlen(target);
        size_t replace_len = strlen(replace);
        size_t origin_len = strlen(original);
        if (target_len == 0 || origin_len == length) {
            return origin_len;
        }

        char *pos = strstr(original, target);
        if (nullptr == pos) {
            return origin_len;
        }

        size_t new_len;
        // There is not enough size after the replacement position to accommodate the string to be replaced.
        if (pos + replace_len > original + length - 1) {
            size_t copy_len = original + length - pos - 1;
            memcpy(pos, replace, copy_len);
            original[length - 1] = '\0';
            new_len = length - 1;
        }
        else {
            // The lengths of target and replace are not equal, so the subsequent string needs to be moved
            if (replace_len != target_len) {
                size_t move_len;
                // If the subsequent length is insufficient, the string after target will be truncated
                if (origin_len + replace_len - target_len > length) {
                    move_len = origin_len - (pos - original) - replace_len;
                }
                else {
                    move_len = origin_len - (pos - original) - target_len;
                }
                memmove(pos + replace_len, pos + target_len, move_len);
                new_len = pos - original + replace_len + move_len;
                original[new_len] = '\0';
            }
            else {
                new_len = origin_len;
            }
            memcpy(pos, replace, replace_len);
        }
        return new_len;
    }

    std::string log4c_replace(const std::string &original, const std::string &target, const std::string &replace) {
        std::string result = original;
        size_t pos = original.find(target);
        if (pos != std::string::npos) {
            result.replace(pos, target.size(), replace);
        }
        return result;
    }

    void get_time_now(tm &now_tm, unsigned short &ms) {
        const auto now = std::chrono::system_clock::now();
        const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        ms = static_cast<unsigned short>(now_ms.count());
    }

    std::string to_lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string to_upper(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
        return result;
    }
}
