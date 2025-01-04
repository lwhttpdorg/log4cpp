#include <atomic>
#include <cstdio>

#include "net_comm.h"

#include "main/log_net.h"

int udpAppenderTest(std::atomic_bool &running, unsigned int log_count, unsigned port) {
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1) {
		printf("socket creation failed...\n");
		return -1;
	}
	const char *hello = "hello";
	const char *bye = "bye";

	sockaddr_in remote_addr = {};
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(remote_addr.sin_addr.s_addr));
	remote_addr.sin_port = htons(port);
	socklen_t socklen = sizeof(remote_addr);
	sendto(fd, hello, strlen(hello), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
	running.store(true);
	char buffer[1024];
	for (unsigned int i = 0; i < log_count; ++i) {
		int len = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (len > 0) {
			buffer[len] = 0;
			printf("UDP[%u]: %s", i, buffer);
		}
		else if (len == -1) {
			break;
		}
	}
	sendto(fd, bye, strlen(bye), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
	close_socket(fd);
	return 0;
}
