#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "Handle.h"
#include "Pool.h"

struct package {
	Pool *pool;
//	Members *mirror;
	int rx_fd;
};

Package *Handle_createPackage(Pool *pool, int rx_fd)
{
	Package *p = malloc(sizeof(*p));
	if (!p) {
		return NULL;
	}
	p->pool = pool;
	p->rx_fd = rx_fd;
	return p;
}

static void *Handle_print(void *arg)
{
	(void)arg;
	printf("We passed something through\n");
	return NULL;
}

void *Handle_request(void *arg)
{
	Package *package = arg;

	if (send(package->rx_fd, "Hello, world!\n", 14, 0) == -1) {
		perror("send");
	}
	close(package->rx_fd);
	Pool_addTask(package->pool, Handle_print, NULL);
	free(package);
	return NULL;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// https://stackoverflow.com/questions/2371910/how-to-get-the-port-number-from-struct-addrinfo-in-unix-c
u_int16_t get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return (((struct sockaddr_in *)sa)->sin_port);
	}
	return (((struct sockaddr_in6 *)sa)->sin6_port);
}
