#include <stdexcept>

#if defined(__linux__)

#include <arpa/inet.h>
#include <netinet/in.h>

#endif

#if defined(_WIN32)
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#endif

#include "common/log_net.hpp"

namespace log4cpp::common {
#ifdef _WIN32
    /**
     * @brief Helper class for Windows Sockets (Winsock) initialization and cleanup.
     *
     * Uses RAII to call WSAStartup() on construction and WSACleanup() on destruction,
     * ensuring Winsock is properly initialized and deinitialized.
     */
    class socket_init {
    public:
        /// @brief Initializes Winsock.
        socket_init() {
            WSADATA wsa_data{};
            (void)WSAStartup(MAKEWORD(2, 2), &wsa_data);
        }

        /// @brief Cleans up Winsock resources.
        ~socket_init() {
            WSACleanup();
        }
    };
#endif
#ifdef _WIN32
    /// @brief Static instance of socket_init to ensure Winsock is initialized once.
    static socket_init net_init{};
#endif

    /**
     * @brief Converts a prefer_stack enum value to its string representation.
     * @param prefer The prefer_stack enum value.
     * @param str The output string to store the result.
     */
    void to_string(prefer_stack prefer, std::string &str) {
        switch (prefer) {
            case prefer_stack::IPv4:
                str = "IPv4";
                break;
            case prefer_stack::IPv6:
                str = "IPv6";
                break;
            case prefer_stack::AUTO:
                str = "AUTO";
                break;
            default:
                // Should not happen with valid enum values.
                str = "Unknown";
                break;
        }
    }

    /**
     * @brief Converts a string to its corresponding prefer_stack enum value.
     *
     * The string comparison is case-insensitive.
     * @param str The input string (e.g., "ipv4", "IPv6", "auto").
     * @param prefer The output prefer_stack enum value.
     * @throws std::invalid_argument if the input string does not match a valid prefer_stack.
     */
    void from_string(const std::string &str, prefer_stack &prefer) {
        const auto prefer_str = to_lower(str);
        if (prefer_str == "ipv4") {
            prefer = prefer_stack::IPv4;
        }
        else if (prefer_str == "ipv6") {
            prefer = prefer_stack::IPv6;
        }
        else if (prefer_str == "auto") {
            prefer = prefer_stack::AUTO;
        }
        else {
            throw std::invalid_argument("Invalid prefer stack string \'" + prefer_str + "\'");
        }
    }

    /// @brief Default constructor for net_addr, initializes to IPv4 with address 0.
    net_addr::net_addr() {
        this->family = net_family::NET_IPv4;
        this->ip.addr4 = 0;
    }

    /**
     * @brief Constructs a net_addr from a C-style string IP address.
     * @param addr The IP address string (e.g., "192.168.1.1", "::1").
     * @throws std::invalid_argument if the address string is not a valid IPv4 or IPv6 address.
     */
    net_addr::net_addr(const char *addr) {
        in_addr addr4{};
        in6_addr addr6{};
        if (0 < inet_pton(AF_INET, addr, &addr4)) {
            this->family = net_family::NET_IPv4;
            this->ip.addr4 = addr4.s_addr;
        }
        else if (0 < inet_pton(AF_INET6, addr, &addr6)) {
            this->family = net_family::NET_IPv6;
            // Use memcpy for a portable, safe copy of the 16-byte IPv6 address.
            std::memcpy(this->ip.addr6, &addr6, sizeof(this->ip.addr6));
        }
        else {
            throw std::invalid_argument("Invalid IP address string \'" + std::string{addr} + "\'");
        }
    }

    /// @brief Constructs a net_addr from a std::string IP address.
    /// @param addr The IP address string.
    net_addr::net_addr(const std::string &addr) : net_addr(addr.c_str()) {
    }

    /**
     * @brief Compares two net_addr objects for equality.
     * @param lhs The left-hand side net_addr.
     * @param rhs The right-hand side net_addr.
     * @return True if the addresses are equal (same family and IP), false otherwise.
     */
    bool operator==(const net_addr &lhs, const net_addr &rhs) {
        if (lhs.family != rhs.family) {
            return false;
        }
        if (lhs.family == net_family::NET_IPv4) {
            return lhs.ip.addr4 == rhs.ip.addr4;
        }
        // For IPv6, compare all 4 32-bit words.
        // Note: This assumes a specific layout for addr6, which might vary slightly across platforms for `in6_addr`.
        return lhs.ip.addr6[0] == rhs.ip.addr6[0] && lhs.ip.addr6[1] == rhs.ip.addr6[1]
               && lhs.ip.addr6[2] == rhs.ip.addr6[2] && lhs.ip.addr6[3] == rhs.ip.addr6[3];
    }

    /**
     * @brief Converts the net_addr to its string representation.
     * @return The IP address as a string.
     * @throws std::invalid_argument if the address family is unknown.
     */
    std::string net_addr::to_string() const {
        std::string s;
        if (this->family == net_family::NET_IPv4) {
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &this->ip.addr4, buf, sizeof(buf));
            s = std::string{buf};
        }
        else if (this->family == net_family::NET_IPv6) {
            char buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &this->ip.addr6, buf, sizeof(buf));
            s = std::string{buf};
        }
        else {
            throw std::invalid_argument("Invalid addr family \'" + std::to_string(static_cast<int>(this->family))
                                        + "\'");
        }
        return s;
    }

    /**
     * @class addrinfo_guard
     * @brief A RAII wrapper for `addrinfo` structures.
     *
     * Ensures that `freeaddrinfo()` is called when the object goes out of scope,
     * preventing memory leaks from `getaddrinfo()`.
     */
    class addrinfo_guard {
    public:
        /// @brief Constructs the guard with a pointer to the `addrinfo` result.
        explicit addrinfo_guard(addrinfo *res) : res_(res) {
        }
        /// @brief Destructor, calls `freeaddrinfo()` if `res_` is not null.
        ~addrinfo_guard() {
            if (res_) {
                freeaddrinfo(res_);
            }
        }
        // Delete copy and move constructors/assignment operators to ensure unique ownership.
        addrinfo_guard(const addrinfo_guard &) = delete;
        addrinfo_guard(addrinfo_guard &&) = delete;
        addrinfo_guard &operator=(const addrinfo_guard &) = delete;
        addrinfo_guard &operator=(addrinfo_guard &&) = delete;

    private:
        /// @brief Pointer to the `addrinfo` structure.
        addrinfo *res_;
    };

    /**
     * @brief Resolves a hostname to an IP address.
     *
     * First checks if the host string is already an IP address. If not, it performs
     * DNS resolution using `getaddrinfo()`, respecting the `prefer_stack` setting.
     * @param host The hostname or IP address string to resolve.
     * @param prefer The preferred IP stack (IPv4, IPv6, or AUTO).
     * @return A `net_addr` object representing the resolved IP address.
     * @throws host_resolve_exception if resolution fails or no valid address is found.
     */
    // Helper function to try parsing an IP address without throwing exceptions.
    bool try_parse(const std::string &host, net_addr &addr) {
        try {
            addr = net_addr(host);
            return true;
        }
        catch (const std::invalid_argument &) {
            return false;
        }
    }

    net_addr net_addr::resolve(const std::string &host, prefer_stack prefer) {
        net_addr addr;
        // First, check if the host string is already a valid IP address.
        if (try_parse(host, addr)) {
            return addr;
        }

        // If not, proceed with DNS resolution.
        addrinfo hints{};
        if (prefer == prefer_stack::IPv4) {
            hints.ai_family = AF_INET;
        }
        else if (prefer == prefer_stack::IPv6) {
            hints.ai_family = AF_INET6;
        }
        else {
            hints.ai_family = AF_UNSPEC;
        }
        // Set socket type to SOCK_STREAM (TCP) as a default hint, though actual protocol is not used for resolution.
        hints.ai_socktype = SOCK_STREAM;

        addrinfo *res = nullptr;
        int ret = getaddrinfo(host.c_str(), nullptr, &hints, &res);
        if (ret != 0) {
#ifdef _WIN32
            throw host_resolve_exception("Failed to resolve host: " + host
                                         + ", error: " + std::to_string(WSAGetLastError()));
#else
            throw host_resolve_exception("Failed to resolve host: " + host + ", error: " + gai_strerror(ret));
#endif
        }
        // Use RAII guard to ensure freeaddrinfo is called.
        addrinfo_guard res_guard(res);

        bool resolved = false;
        for (const addrinfo *p = res; p != nullptr; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                const sockaddr_in *addr4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
                addr.family = net_family::NET_IPv4;
                addr.ip.addr4 = addr4->sin_addr.s_addr;
                resolved = true;
                break;
            }
            if (p->ai_family == AF_INET6) {
                const sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
                addr.family = net_family::NET_IPv6;
                std::memcpy(addr.ip.addr6, addr6->sin6_addr.s6_addr, sizeof(addr.ip.addr6));
                resolved = true;
                break;
            }
        }
        if (!resolved) {
            throw host_resolve_exception("No valid address found for host: " + host);
        }
        return addr;
    }

    /**
     * @brief Serializes a net_addr object to JSON.
     * @param j The JSON object to write to.
     * @param addr The net_addr object to serialize.
     */
    void to_json(nlohmann::json &j, const net_addr &addr) {
        j = addr.to_string();
    }

    /**
     * @brief Deserializes a net_addr object from JSON.
     * @param j The JSON object to read from.
     * @param addr The net_addr object to populate.
     */
    void from_json(const nlohmann::json &j, net_addr &addr) {
        const std::string s = j.get<std::string>();
        addr = net_addr(s);
    }
}
