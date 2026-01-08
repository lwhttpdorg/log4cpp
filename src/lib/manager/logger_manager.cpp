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

    /**
     * @enum EVENT_TYPE
     * @brief Defines the event types for the non-Windows event loop.
     */
    enum EVENT_TYPE { EVT_HOT_RELOAD = 1, EVT_SHUTDOWN = 2 };

    /// @brief A flag to ensure thread-safe initialization of the logger_manager singleton.
    std::once_flag logger_manager::init_flag{};
    /// @brief The unique static instance of the logger_manager.
    logger_manager logger_manager::instance;

#ifndef _WIN32
    /**
     * @brief The signal handler for the supervisor, used to receive hot-reload signals.
     * @param sig_num The received signal number (unused).
     */
    void supervisor::sigusr2_handle([[maybe_unused]] int sig_num) {
#ifdef _DEBUG
        common::log4c_debug(stdout, "[sigusr2_handle] log4pp received hot reload trigger event\n");
#endif
        const logger_manager &logger_mgr = get_logger_manager();
        logger_mgr.notify_config_hot_reload();
    }

    /**
     * @brief Enables the configuration hot-reloading feature.
     * @param sig The signal to use for triggering the hot-reload (defaults to SIGHUP).
     * @return True if the signal handler was set and the event loop thread was started successfully.
     */
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

    /**
     * @class logger_deleter
     * @brief A custom deleter for std::shared_ptr<logger_proxy>.
     *
     * This deleter is invoked when the last shared_ptr to a logger_proxy is destroyed.
     * Its core responsibility is to notify the logger_manager to remove the corresponding
     * logger record from its map of active loggers, and then to safely delete the
     * logger_proxy object itself. This ensures that the logger_manager does not hold
     * dangling weak pointers to destroyed objects.
     */
    class logger_deleter {
    public:
        /**
         * @brief Constructs the deleter.
         * @param name The name of the logger to be deleted.
         */
        explicit logger_deleter(std::string name) : logger_name(std::move(name)) {
        }

        /**
         * @brief The deleter call operator.
         * @param logger_ptr A pointer to the logger_proxy object to be deleted.
         */
        void operator()(const logger_proxy *logger_ptr) const {
            // Deregister this logger from the manager.
            auto &log_mgr = supervisor::get_logger_manager();
            log_mgr.release_logger(this->logger_name);
            // Delete the proxy object.
            delete logger_ptr;
        }

    private:
        /// @brief The name of the logger to be deleted.
        std::string logger_name;
    };
    // ========================================
    // logger manager
    // ========================================

    /// @brief Constructor, initializes member variables.
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

    /// @brief Destructor, responsible for cleaning up resources like closing the eventfd and joining the event loop
    /// thread.
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

    /**
     * @brief Automatically loads the configuration.
     *
     * Attempts to load the configuration file from the default path. If it fails (e.g., file not found or parse error),
     * it constructs a built-in default configuration.
     */
    void logger_manager::auto_load_config() {
        std::filesystem::path fsp(config_file_path);

        if (exists(fsp)) {
            try {
                std::unique_lock writer_lock(config_rw_lock);
                load_config(config_file_path);
                return;
            }
            catch (const std::exception &e) {
                log4cpp::common::log4c_debug(
                    stderr, "Failed to load the configuration file automatically, using default configuration. [%s]\n",
                    e.what());
            }
        }

        this->config = std::make_unique<config::log4cpp>();
        this->config->log_pattern = DEFAULT_LOG_PATTERN;
#if __cplusplus >= 202002L
        this->config->appenders.console = config::console_appender{.out_stream = "stdout"};
        const config::logger fallback_logger{.name = FALLBACK_LOGGER_NAME,
                                             .level = log_level::WARN,
                                             .appender = static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)};
#else
        this->config->appenders.console = config::console_appender{"stdout"};
        const config::logger fallback_logger{FALLBACK_LOGGER_NAME, log_level::WARN,
                                             static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)};
#endif
        this->config->loggers.emplace(fallback_logger.name, fallback_logger);
    }

    /**
     * @brief Loads the configuration from a specified file path.
     * @param file_path The path to the configuration file.
     * @throws std::runtime_error If the file cannot be opened.
     */
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
    /**
     * @brief The main function for the event loop thread.
     *
     * This loop blocks on a read() call, waiting for events from the signal handler or the destructor.
     */
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
                        common::log4c_debug(stdout, "%s:%d, received unknown event %lu\n", __func__, __LINE__, event);
#endif
                        break;
                }
            }
            else if (s == -1) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno != EBADF) {
                    common::log4c_debug(stderr, "%s: event fd read error! break event loop!\n", __func__);
                }
                break;
            }
        }
    }

    /// @brief Sends a hot-reload notification to the event loop thread.
    void logger_manager::notify_config_hot_reload() const {
        constexpr uint64_t val = EVT_HOT_RELOAD;
        (void)write(evt_fd, &val, sizeof(uint64_t));
    }

    /// @brief Creates and starts the event loop thread.
    void logger_manager::start_hot_reload_thread() {
        evt_loop_run.store(true);
        evt_fd = eventfd(0, 0);
        evt_loop_thread = std::thread(&logger_manager::event_loop, this);
    }

    /**
     * @brief Executes the configuration hot-reload.
     *
     * It reloads the configuration file, compares it with the old configuration,
     * and then rebuilds Appenders and updates all active Loggers based on the differences.
     * The entire process is thread-safe.
     */
    void logger_manager::hot_reload_config() {
#ifdef _DEBUG
        common::log4c_debug(stdout, "[logger_manager] hot_reload_config\n");
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
                common::log4c_debug(stderr, "[%s:%d] failed to reload config: %s\n", __func__, __LINE__, e.what());
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

    /**
     * @brief Updates all active loggers after a hot-reload.
     *
     * This function calculates the diff of logger configurations (added, removed, changed)
     * by comparing the new and old configs. It then iterates over all currently active
     * logger proxies and updates their underlying real logger instances as needed.
     * @param old_log_cfg The unordered_map of logger configurations before the hot-reload.
     * @param appender_chg A boolean indicating if the Appender configuration has changed.
     */
    void logger_manager::update_logger(const std::unordered_map<std::string, config::logger> &old_log_cfg,
                                       bool appender_chg) {
        const std::unordered_map<std::string, config::logger> &new_log_cfg = this->config->loggers;

        // Calculate the diff: added, removed, changed
        std::unordered_set<std::string> added, removed, changed;
        for (const auto &[name, old_log]: old_log_cfg) {
            if (new_log_cfg.find(name) == new_log_cfg.end()) {
                // Not in new config, so it was removed
                removed.insert(name);
            }
            else {
                // In both configs, check if they are different
                const config::logger &new_cfg = new_log_cfg.at(name);
                if (new_cfg != old_log) {
                    changed.insert(name);
                }
            }
        }
        for (auto &[name, new_log]: new_log_cfg) {
            if (old_log_cfg.find(name) == old_log_cfg.end()) {
                // In new config but not in old config, so it was added
                added.insert(name);
            }
        }

        // Get the default FALLBACK_LOGGER_NAME config
        const config::logger &new_fallback_logger = new_log_cfg.at(FALLBACK_LOGGER_NAME);
        const config::logger &old_fallback_logger = old_log_cfg.at(FALLBACK_LOGGER_NAME);
        bool fallback_chged = old_fallback_logger != new_fallback_logger;
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
                bool cfg_chged = false;
                if (changed.find(logger_name) != changed.end() || added.find(logger_name) != added.end()
                    || removed.find(logger_name) != removed.end()) {
                    cfg_chged = true;
                }
                bool is_fallback = false;
                const auto old_it = old_log_cfg.find(logger_name);
                if (old_log_cfg.end() == old_it) {
                    is_fallback = true;
                }
                if (is_fallback && fallback_chged) {
                    cfg_chged = true;
                }
                if (appender_chg || cfg_chged) {
                    std::shared_ptr<logger> new_logger = nullptr;
                    if (new_log_cfg.find(logger_name) != new_log_cfg.end()) {
                        new_logger = build_logger(new_log_cfg.at(logger_name));
                    }
                    else {
                        new_logger = build_logger(new_fallback_logger);
                    }
                    proxy->set_target(new_logger);
                }
                ++it;
            }
        }
    }
#endif
    std::shared_ptr<logger> logger_manager::get_logger(const std::string &name) {
        // On the first call, use std::call_once to ensure thread-safe initialization of the singleton.
        // The initialization process includes:
        // 1. Automatically loading the configuration (auto_load_config)
        // 2. Setting the log pattern (set_log_pattern)
        // 3. Building the appenders (build_appender)
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

    /**
     * @brief Gets or creates a logger instance (lazy initialization).
     *
     * This is the core of logger management. It uses the Double-Checked Locking Pattern
     * to efficiently and thread-safely get or create a logger.
     * 1. Fast Path: Uses a read lock (shared_lock) to check if the logger already exists. If so, returns it.
     * 2. Slow Path: If it doesn't exist, upgrades to a write lock (unique_lock), checks again (to prevent a race
     * condition), then creates the new logger and proxy.
     * @param name The name of the logger.
     * @return A shared pointer to the logger_proxy.
     */
    std::shared_ptr<logger_proxy> logger_manager::get_or_create_logger(const std::string &name) {
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

        const config::logger fallback_log_cfg = this->config->loggers[FALLBACK_LOGGER_NAME];
        config::logger log_cfg;
        auto cfg_it = this->config->loggers.find(name);
        if (this->config->loggers.end() != cfg_it) {
            log_cfg = cfg_it->second;
            if (!log_cfg.level.has_value()) {
                log_cfg.level = fallback_log_cfg.level;
            }
            if (0 == log_cfg.appender) {
                log_cfg.appender = fallback_log_cfg.appender;
            }
        }
        else {
            log_cfg = fallback_log_cfg;
        }

        auto new_logger = build_logger(log_cfg);

        // Set the unique name for the new logger.
        new_logger->set_name(name);

        // Wrap the new logger in a proxy object
        const auto proxy = std::shared_ptr<logger_proxy>(new logger_proxy(new_logger), logger_deleter{name});

        // Insert the proxy into the map under the protection of the unique lock.
        loggers[name] = proxy;

        // Return the newly created proxy.
        return proxy;
    }

    /// @brief Removes a logger from the logger map. This is called by the logger_deleter.
    void logger_manager::release_logger(const std::string &name) {
        std::unique_lock lock(logger_rw_lock);
        loggers.erase(name);
    }

    /// @brief Sets the global log pattern based on the configuration.
    void logger_manager::set_log_pattern() const {
        if (config->log_pattern.has_value()) {
            pattern::log_pattern::set_pattern(config->log_pattern.value());
        }
        else {
            pattern::log_pattern::set_pattern(DEFAULT_LOG_PATTERN);
        }
    }

    /**
     * @brief Builds all Appenders that are referenced in the configuration.
     *
     * This is a lazy-loading implementation: it first iterates through all logger
     * configurations to determine which appender types are necessary, then creates only those that are actually used.
     */
    void logger_manager::build_appender() {
        // Determine which appenders are actually required by iterating through all loggers.
        unsigned char required_appenders_mask = 0;
        if (config && !config->loggers.empty()) {
            for (const auto &[name, log]: config->loggers) {
                required_appenders_mask |= log.appender;
            }
        }

        const config::log_appender &appender_cfg = config->appenders;
        std::shared_ptr<appender::log_appender> new_console_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_file_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_socket_appender = nullptr;

        // Lazily create appenders only if they are defined AND required by a logger.
        if ((required_appenders_mask & static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE))
            && appender_cfg.console.has_value()) {
            new_console_appender = std::make_shared<appender::console_appender>(appender_cfg.console.value());
        }
        if ((required_appenders_mask & static_cast<unsigned char>(config::APPENDER_TYPE::FILE))
            && appender_cfg.file.has_value()) {
            new_file_appender = std::make_shared<appender::file_appender>(appender_cfg.file.value());
        }
        if ((required_appenders_mask & static_cast<unsigned char>(config::APPENDER_TYPE::SOCKET))
            && appender_cfg.socket.has_value()) {
            new_socket_appender = std::make_shared<appender::socket_appender>(appender_cfg.socket.value());
        }

        std::unique_lock lock(appender_rw_lock);
        this->console_appender_ptr = new_console_appender;
        this->file_appender_ptr = new_file_appender;
        this->socket_appender_ptr = new_socket_appender;
    }

    /**
     * @brief Builds a concrete core_logger instance based on the given configuration.
     * @param log_cfg The configuration for this logger.
     * @return A shared pointer to the newly created core_logger.
     */
    std::shared_ptr<logger> logger_manager::build_logger(const config::logger &log_cfg) const {
        std::shared_ptr<appender::log_appender> temp_appenders[3];

        std::shared_lock appender_lock(appender_rw_lock);
        temp_appenders[0] = this->console_appender_ptr;
        temp_appenders[1] = this->file_appender_ptr;
        temp_appenders[2] = this->socket_appender_ptr;

        auto new_logger = std::make_shared<core_logger>(log_cfg.name, log_cfg.level.value());
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
