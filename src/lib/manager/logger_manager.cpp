#include <cstdio>
#include <filesystem>
#include <fstream>
#include <utility>

#ifndef _WIN32
#include <sys/eventfd.h>
#include <unordered_set>
#endif

#include <nlohmann/json.hpp>

#include <log4cpp/log4cpp.hpp>
#include <log4cpp/logger.hpp>
#include <logger/core_logger.hpp>

#include "appender/console_appender.hpp"
#include "config/log4cpp.hpp"
#include "pattern/log_pattern.hpp"

#ifdef _WIN32
#include <io.h>
#endif

#include <appender/file_appender.hpp>
#include <appender/socket_appender.hpp>

constexpr const char *DEFAULT_CONFIG_FILE_PATH = "./log4cpp.json";

namespace log4cpp {

    enum EVENT_TYPE { EVT_HOT_RELOAD = 1, EVT_SHUTDOWN = 2 };

    std::once_flag logger_manager::init_flag{};
    logger_manager logger_manager::instance;

#ifndef _WIN32
    void supervisor::sigusr2_handle([[maybe_unused]] int sig_num) {
#ifdef _DEBUG
        printf("%s:%d, log4pp received hot reload trigger event\n", __func__, __LINE__);
#endif
        const logger_manager &logger_mgr = get_logger_manager();
        logger_mgr.notify_config_hot_reload();
    }

    bool supervisor::enable_config_hot_loading(int sig) {
        struct sigaction sa{};

        sa.sa_handler = sigusr2_handle;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(sig, &sa, nullptr) == -1) {
            return false;
        }
        logger_manager &logger_mgr = get_logger_manager();
        logger_mgr.start_hot_reload_thread();
        return true;
    }
#endif

    logger_manager &supervisor::get_logger_manager() {
        return logger_manager::instance;
    }

    std::string supervisor::serialize(const config::log4cpp &cfg) {
        return config::log4cpp::serialize(cfg);
    }

    class logger_deleter {
    public:
        explicit logger_deleter(std::string name) : logger_name(std::move(name)) {
        }

        void operator()(const log::logger_proxy *logger_ptr) const {
            auto &log_mgr = supervisor::get_logger_manager();
            log_mgr.release_logger(this->logger_name);
            delete logger_ptr;
        }

    private:
        std::string logger_name;
    };
    // ========================================
    // logger manager
    // ========================================

    logger_manager::logger_manager() {
#ifndef _WIN32
        evt_fd = -1;
        evt_loop_run.store(false);
#endif
        config_file_path = DEFAULT_CONFIG_FILE_PATH;
        console_appender_ptr = nullptr;
        file_appender_ptr = nullptr;
        socket_appender_ptr = nullptr;
    }

    logger_manager::~logger_manager() {
#ifndef _WIN32
        if (evt_loop_thread.joinable()) {
            evt_loop_run.store(false);
            constexpr uint64_t val = EVT_SHUTDOWN;
            (void)write(evt_fd, &val, sizeof(uint64_t));
            evt_loop_thread.join();
        }
        if (evt_fd != -1) {
            close(evt_fd);
        }
#endif
    }

    void logger_manager::auto_load_config() {
        std::filesystem::path fsp(config_file_path);

        if (exists(fsp)) {
            try {
                std::unique_lock writer_lock(config_rw_lock);
                load_config(config_file_path);
                return;
            }
            catch (const std::exception &e) {
                fprintf(stderr,
                        "Failed to load the configuration file automatically, using default configuration. [%s]\n",
                        e.what());
                fflush(stderr);
            }
        }

        this->config = std::make_unique<config::log4cpp>();
        this->config->log_pattern = DEFAULT_LOG_PATTERN;
#if __cplusplus >= 202002L
        this->config->appenders.console = config::console_appender{.out_stream = "stdout"};
        const config::logger default_logger{.name = "root",
                                            .level = log_level::WARN,
                                            .appender = static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)};
#else
        this->config->appenders.console = config::console_appender{"stdout"};
        const config::logger default_logger{"root", log_level::WARN,
                                            static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)};
#endif
        this->config->loggers.push_back(default_logger);
    }

    void logger_manager::load_config(const std::string &file_path) {
        if (std::filesystem::exists(file_path)) {
            std::ifstream ifs(file_path);
            if (!ifs.is_open()) {
                throw std::runtime_error("cannot open config file: " + file_path);
            }
            nlohmann::json j;
            ifs >> j;

            config = std::make_unique<config::log4cpp>(j.get<config::log4cpp>());
            config_file_path = file_path;
        }
        else {
            throw std::filesystem::filesystem_error("Config file " + file_path + " open failed!",
                                                    std::make_error_code(std::io_errc::stream));
        }
    }
#ifndef _WIN32
    void logger_manager::event_loop() {
        set_thread_name("event_loop");
        uint64_t event;

        while (evt_loop_run.load()) {
            ssize_t s = read(evt_fd, &event, sizeof(uint64_t));
            if (s == sizeof(uint64_t)) {
                switch (event) {
                    case EVT_HOT_RELOAD:
                        hot_reload_config();
                        break;
                    case EVT_SHUTDOWN:
                        break;
                    default:
#ifdef _DEBUG
                        fprintf(stderr, "%s:%d, received unknown event %lu\n", __func__, __LINE__, event);
                        fflush(stderr);
#endif
                        break;
                }
            }
            else if (s == -1) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno != EBADF) {
                    fprintf(stderr, "%s: event fd read error! break event loop!\n", __func__);
                    fflush(stderr);
                }
                break;
            }
        }
    }

    void logger_manager::notify_config_hot_reload() const {
        constexpr uint64_t val = EVT_HOT_RELOAD;
        (void)write(evt_fd, &val, sizeof(uint64_t));
    }

    void logger_manager::start_hot_reload_thread() {
        evt_loop_run.store(true);
        evt_fd = eventfd(0, 0);
        evt_loop_thread = std::thread(&logger_manager::event_loop, this);
    }

    void logger_manager::hot_reload_config() {
#ifdef _DEBUG
        printf("[logger_manager] hot_reload_config\n");
#endif
        config::log4cpp old_cfg;
        bool appenders_changed = false;
        {
            std::unique_lock writer_lock(config_rw_lock);
            old_cfg = *this->config;
            try {
                load_config(config_file_path);
            }
            catch (const std::exception &e) {
                fprintf(stderr, "[%s:%d] failed to reload config: %s\n", __func__, __LINE__, e.what());
                fflush(stderr);
                return;
            }
            if (old_cfg == *this->config) {
                return;
            }
            if (old_cfg.appenders != this->config->appenders) {
                appenders_changed = true;
            }
        }
        set_log_pattern();
        build_appender();
        update_logger(old_cfg.loggers, appenders_changed);
    }

    void logger_manager::update_logger(const std::vector<config::logger> &old_log_cfg, bool appender_chg) {
        // Build hashmaps for efficient O(1) lookups
        std::unordered_map<std::string, config::logger> new_log_cfg_hash, old_log_cfg_hash;
        for (const auto &log_cfg: this->config->loggers) {
            new_log_cfg_hash[log_cfg.name] = log_cfg;
        }
        for (const auto &log_cfg: old_log_cfg) {
            old_log_cfg_hash[log_cfg.name] = log_cfg;
        }

        // Calculate the diff: added, removed, changed
        std::unordered_set<std::string> added, removed, changed;
        for (const auto &old_cfg: old_log_cfg) {
            if (new_log_cfg_hash.find(old_cfg.name) == new_log_cfg_hash.end()) {
                // Not in new config, so it was removed
                removed.insert(old_cfg.name);
            }
            else {
                // In both configs, check if they are different
                auto new_cfg = new_log_cfg_hash[old_cfg.name];
                if (new_cfg != old_cfg) {
                    changed.insert(old_cfg.name);
                }
            }
        }
        for (auto &new_cfg: new_log_cfg_hash) {
            if (old_log_cfg_hash.find(new_cfg.first) == old_log_cfg_hash.end()) {
                // In new config but not in old config, so it was added
                added.insert(new_cfg.first);
            }
        }

        // Get the default "root" config, which is assumed to be the first one
        const config::logger &root_log_cfg = this->config->loggers.front();
        /*
         * 'this->loggers' stores the currently active (in-use) logger proxies.
         * We must iterate over this map and update the target of each proxy
         * based on the configuration changes.
         *
         * 1. For ADDED loggers:
         * Build a new logger with the new config and set it as the proxy's target.
         * (This handles the edge case where a logger was previously created
         * using the default/root config and now has its own specific config).
         *
         * 2. For CHANGED loggers:
         * Build a new logger with the *updated* config and set it as the proxy's target.
         *
         * 3. For REMOVED loggers:
         * Build a new logger with the *default* ("root") config and set it as the proxy's target.
         * (We don't remove the proxy, we just revert it to the default behavior).
         * 4. For ALL loggers
         * If 'appender_chg' is true, a rebuild is mandatory, regardless of local config status.
         */
        std::unique_lock writer_lock(logger_rw_lock);
        for (auto it = loggers.begin(); it != loggers.end();) {
            const auto logger_name = it->first;
            auto proxy_weak_ptr = it->second;
            auto proxy = proxy_weak_ptr.lock();
            if (nullptr == proxy) {
                it = loggers.erase(it);
            }
            else {
                bool log_changed = false;
                if (changed.find(logger_name) != changed.end() || added.find(logger_name) != added.end()
                    || removed.find(logger_name) != removed.end()) {
                    log_changed = true;
                }
                if (appender_chg || log_changed) {
                    std::shared_ptr<log::logger> new_logger = nullptr;
                    if (new_log_cfg_hash.find(logger_name) != new_log_cfg_hash.end()) {
                        new_logger = build_logger(new_log_cfg_hash[logger_name]);
                    }
                    else {
                        new_logger = build_logger(root_log_cfg);
                    }
                    proxy->set_target(new_logger);
                }
                ++it;
            }
        }
    }
#endif
    std::shared_ptr<log::logger> logger_manager::get_logger(const std::string &name) {
        std::call_once(init_flag, [] {
            if (nullptr == instance.config) {
                instance.auto_load_config();
            }
            instance.set_log_pattern();
            instance.build_appender();
        });
        return instance.get_or_create_logger(name);
    }

    const config::log4cpp *logger_manager::get_config() const {
        return this->config.get();
    }

    // Lazy Initialization
    std::shared_ptr<log::logger_proxy> logger_manager::get_or_create_logger(const std::string &name) {
        // Acquire a shared lock (read lock) for the fast path.
        {
            std::shared_lock reader_lock(logger_rw_lock);
            auto it = loggers.find(name);
            // First check: If the logger exists, return it immediately.
            if (it != loggers.end()) {
                auto log_ptr = it->second.lock();
                if (log_ptr != nullptr) {
                    return log_ptr;
                }
            }
        } // reader_lock is automatically released here.

        // Acquire an exclusive lock (write lock) for the slow path (creation).
        std::unique_lock writer_lock(logger_rw_lock);

        // Second check: Must check again to prevent a race condition
        // where another thread created the logger while this thread was waiting for the lock.
        auto it = loggers.find(name);
        if (it != loggers.end()) {
            auto log_ptr = it->second.lock();
            if (log_ptr != nullptr) {
                return log_ptr;
            }
            loggers.erase(it);
        }

        // --- Logger Creation ---

        std::vector<config::logger>::iterator cfg_it;
        for (cfg_it = this->config->loggers.begin(); cfg_it != this->config->loggers.end(); ++cfg_it) {
            if (cfg_it->name == name) {
                break;
            }
        }

        // "root" logger is the first one
        const config::logger root_log_cfg = this->config->loggers.front();
        config::logger log_cfg;
        if (cfg_it != this->config->loggers.end()) {
            log_cfg = *cfg_it;
            if (!log_cfg.level.has_value()) {
                log_cfg.level = root_log_cfg.level;
            }
            if (0 == log_cfg.appender) {
                log_cfg.appender = root_log_cfg.appender;
            }
        }
        else {
            log_cfg = root_log_cfg;
        }

        auto new_logger = build_logger(log_cfg);

        // Set the unique name for the new logger.
        new_logger->set_name(name);

        // Wrap the new logger in a proxy object
        const auto proxy = std::shared_ptr<log::logger_proxy>(new log::logger_proxy(new_logger), logger_deleter{name});

        // Insert the proxy into the map under the protection of the unique lock.
        loggers[name] = proxy;

        // Return the newly created proxy.
        return proxy;
    }

    void logger_manager::release_logger(const std::string &name) {
        std::unique_lock lock(logger_rw_lock);
        loggers.erase(name);
    }

    void logger_manager::set_log_pattern() const {
        if (config->log_pattern.has_value()) {
            pattern::log_pattern::set_pattern(config->log_pattern.value());
        }
        else {
            pattern::log_pattern::set_pattern(DEFAULT_LOG_PATTERN);
        }
    }

    void logger_manager::build_appender() {
        const config::log_appender &appender_cfg = config->appenders;
        std::shared_ptr<appender::log_appender> new_console_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_file_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_socket_appender = nullptr;
        if (appender_cfg.console.has_value()) {
            new_console_appender = std::make_shared<appender::console_appender>(appender_cfg.console.value());
        }
        if (appender_cfg.file.has_value()) {
            new_file_appender = std::make_shared<appender::file_appender>(appender_cfg.file.value());
        }
        if (appender_cfg.socket.has_value()) {
            new_socket_appender = std::make_shared<appender::socket_appender>(appender_cfg.socket.value());
        }

        std::unique_lock lock(appender_rw_lock);
        this->console_appender_ptr = new_console_appender;
        this->file_appender_ptr = new_file_appender;
        this->socket_appender_ptr = new_socket_appender;
    }

    std::shared_ptr<log::logger> logger_manager::build_logger(const config::logger &log_cfg) const {
        std::shared_ptr<appender::log_appender> temp_appenders[3];

        std::shared_lock appender_lock(appender_rw_lock);
        temp_appenders[0] = this->console_appender_ptr;
        temp_appenders[1] = this->file_appender_ptr;
        temp_appenders[2] = this->socket_appender_ptr;

        auto new_logger = std::make_shared<log::core_logger>(log_cfg.name, log_cfg.level.value());
        if (log_cfg.appender & static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)) {
            new_logger->add_appender(temp_appenders[0]);
        }
        if (log_cfg.appender & static_cast<unsigned char>(config::APPENDER_TYPE::FILE)) {
            new_logger->add_appender(temp_appenders[1]);
        }
        if (log_cfg.appender & static_cast<unsigned char>(config::APPENDER_TYPE::SOCKET)) {
            new_logger->add_appender(temp_appenders[2]);
        }
        return new_logger;
    }
}
