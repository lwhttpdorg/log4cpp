#include <mutex> // for std::scoped_lock

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h> // for write, STDOUT_FILENO
#endif
#ifdef _WIN32
#include <io.h>      // for _write, _fileno
#include <windows.h> // for Windows API
#endif

#include "appender/console_appender.hpp" // for console_appender

#ifdef _MSC_VER
#define STDOUT_FILENO _fileno(stdout)
#define STDERR_FILENO _fileno(stderr)
#endif

namespace log4cpp::appender {
    int stream_name_to_file_no(const std::string &out_stream) {
        int file_no;
        if (out_stream == "stdout") {
            file_no = STDOUT_FILENO;
        }
        else if (out_stream == "stderr") {
            file_no = STDERR_FILENO;
        }
        else {
            throw std::invalid_argument("Invalid 'console_appender' out_stream \'" + out_stream
                                        + "\', valid name: stdout, stderr");
        }
        return file_no;
    }

    console_appender::console_appender(const config::console_appender &cfg) {
        this->file_no = stream_name_to_file_no(cfg.out_stream);
    }

    void console_appender::log(const char *msg, size_t msg_len) {
        std::scoped_lock fd_lock(this->lock);
#ifdef _MSC_VER
        (void)_write(this->file_no, msg, static_cast<unsigned int>(msg_len));
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        (void)write(this->file_no, msg, msg_len);
#pragma GCC diagnostic pop
#endif
    }
} // namespace log4cpp::appender
