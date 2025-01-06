#pragma once

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#if defined(_MSC_VER) || defined(_WIN32)
constexpr SOCKET INVALID_FD = INVALID_SOCKET;
typedef SOCKET socket_fd;

inline void close_socket(socket_fd fd) {
	closesocket(fd);
}
#endif
#if defined(__linux__)
constexpr int INVALID_FD = -1;
typedef int socket_fd;

inline void close_socket(socket_fd fd) {
	close(fd);
}

#endif

#ifdef _WIN32
// Windows socket initialization
class winsock_init {
public:
	winsock_init() {
		WSADATA wsa_data{};
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
	}

	~winsock_init() {
		WSACleanup();
	}
};
#endif
