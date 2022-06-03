#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

long vscnprintf(char *buf, ssize_t size, const char *fmt, va_list args)
{
	int i = vsnprintf(buf, size, fmt, args);
	ssize_t ssize = size;
	return (i >= ssize)?(ssize - 1):i;
}

long scnprintf(char *buf, ssize_t size, const char *fmt, ...)
{
	ssize_t ssize = size;
	va_list args;
	int i;
	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return (i >= ssize)?(ssize - 1):i;
}

unsigned long readline(int fd, char *buf, unsigned long size)
{
	unsigned long read_size = 0;
	char ch = '\0';
	long recv_len;
	while ((read_size < size - 1) && (ch != '\n'))
	{
		recv_len = recv(fd, &ch, 1, 0);
		if (recv_len > 0)
		{
			if (ch == '\r')
			{
				recv_len = recv(fd, &ch, 1, MSG_PEEK);
				if ((recv_len > 0) && (ch == '\n'))
				{
					recv(fd, &ch, 1, 0);
				}
				else
				{
					ch = '\n';
				}
			}
			buf[read_size++] = ch;
		}
		else
		{
			ch = '\n';
		}
	}
	buf[read_size] = '\0';
	return read_size;
}

void handle_error(const char *func_name, int line, int error_code, int fd, const char *msg)
{
	if (fd != -1)
	{
		close(fd);
	}
	printf("%s@%d: %s, %s(%d)", func_name, line, msg, strerror(error_code), error_code);
}
