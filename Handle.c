#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "Handle.h"
#include "Pool.h"
#include "Mirrors.h"

struct package {
	Pool *pool;
	Mirrors *mirrors;
	int rx_fd;
};

Package *Handle_createPackage(Pool *pool, Mirrors *mirrors, int rx_fd)
{
	Package *p = malloc(sizeof(*p));
	if (!p) {
		return NULL;
	}
	p->pool = pool;
	p->mirrors = mirrors;
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

	struct sockaddr_storage *addr = malloc(sizeof(*addr));
	socklen_t addr_sz = sizeof(struct sockaddr_storage);
	getpeername(package->rx_fd, (struct sockaddr *)addr, &addr_sz);

	Mirrors_add(package->mirrors, addr);
	Mirrors_rm(package->mirrors, addr);
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
