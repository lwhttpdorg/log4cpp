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
    // Platform-specific implementations for thread name management.
    /**
     * @brief Gets the name and ID of the current thread in a platform-independent way.
     * @param[out] thread_name Buffer to store the retrieved thread name.
     * @param len The size of the thread_name buffer.
     * @return The current thread's ID.
     */
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

    /**
     * @brief Sets the name of the current thread in a platform-independent way.
     * @param name The desired name for the thread.
     */
    void set_thread_name(const char *name) {
#ifdef _MSC_VER
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        std::wstring wchar_str(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, name, -1, &wchar_str[0], size_needed);
        SetThreadDescription(GetCurrentThread(), wchar_str.c_str());
#endif
#ifdef __GNUC__
        char buf[16]; // pthread_setname_np requires a buffer of 16 chars max.
        common::log4c_scnprintf(buf, sizeof(buf), "%s", name);
        pthread_setname_np(pthread_self(), buf);
#elif __linux__
        char buf[16]; // prctl(PR_SET_NAME) has a limit of 16 bytes.
        common::log4c_scnprintf(buf, sizeof(buf), "%s", name);
        prctl(PR_SET_NAME, buf);
#endif
    }
}

namespace log4cpp::common {
    /**
     * @brief A safe wrapper around vsnprintf.
     *
     * @param buf The buffer to write to.
     * @param size The size of the buffer.
     * @param fmt The format string.
     * @param args The va_list of arguments.
     * @return The number of characters written, or 0 on error.
     */
    size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
        int i = vsnprintf(buf, size, fmt, args);
        if (i > 0) {
            return i;
        }
        return 0;
    }

    /**
     * @brief A safe wrapper around snprintf with variadic arguments.
     *
     * @param buf The buffer to write to.
     * @param size The size of the buffer.
     * @param fmt The format string.
     * @param ...
     * @return The number of characters written, or 0 on error.
     */
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

    /**
     * @brief A simple internal debug logging function.
     *
     * Prints a timestamped debug message to the specified stream.
     */
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

    /**
     * @brief A safe, in-place string replacement function for C-style strings.
     *
     * Replaces the first occurrence of `target` with `replace` in the `original` buffer.
     * Handles buffer size limits to prevent overflows.
     * @param original The buffer containing the string to modify.
     * @param length The total size of the `original` buffer.
     * @param target The substring to search for.
     * @param replace The string to replace `target` with.
     * @return The new length of the modified string in the buffer.
     */
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

    /**
     * @brief A simple string replacement function for std::string.
     *
     * Replaces the first occurrence of `target` with `replace` in the given string.
     * @param original The original string.
     * @param target The substring to search for.
     * @param replace The string to replace `target` with.
     * @return A new string with the replacement made.
     */
    std::string log4c_replace(const std::string &original, const std::string &target, const std::string &replace) {
        std::string result = original;
        size_t pos = original.find(target);
        if (pos != std::string::npos) {
            result.replace(pos, target.size(), replace);
        }
        return result;
    }

    /**
     * @brief Gets the current system time with millisecond precision.
     *
     * @param[out] now_tm The output `tm` struct representing the current time.
     * @param[out] ms The output millisecond part of the current time.
     */
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

    /**
     * @brief Converts a string to lowercase.
     * @param s The input string to convert.
     * @return The converted lowercase string.
     */
    std::string to_lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    /**
     * @brief Converts a string to uppercase.
     * @param s The input string to convert.
     * @return The converted uppercase string.
     */
    std::string to_upper(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
}
