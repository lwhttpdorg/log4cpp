#include <atomic>
#include <cstdio>

#include "net_comm.h"

#include "main/log_net.h"

int tcpAppenderTest(std::atomic_bool &running, unsigned int log_count, unsigned port) {
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		printf("socket creation failed...\n");
		return -1;
	}

	sockaddr_in remote_addr = {};
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(remote_addr.sin_addr.s_addr));
	remote_addr.sin_port = htons(port);
	socklen_t socklen = sizeof(remote_addr);
	int val = connect(fd, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
	if (-1 == val) {
#ifdef _WIN32
		int wsa_error = WSAGetLastError();
		printf("%s,L%d,connect failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
		printf("%s,L%d,connect failed! errno:%d,%s\n", __func__, __LINE__, errno, strerror(errno));
#endif
		return -2;
	}
	running.store(true);
	char buffer[1024];
	for (unsigned int i = 0; i < log_count; i++) {
		int len = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (len > 0) {
			buffer[len] = 0;
			printf("TCP[%u]: %s", i, buffer);
		}
		else if (len == -1) {
			break;
		}
	}

#ifdef _WIN32
	shutdown(fd, SD_BOTH);
#else
    shutdown(fd, SHUT_RDWR);
#endif
	close_socket(fd);
	return 0;
}
