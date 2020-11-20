#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "Handle.h"
#include "Pool.h"
#include "Mirrors.h"

struct package {
	Pool *pool;
	Mirrors *mirrors;
	int rx_fd;
	union {
		char *message;
		uint64_t ascii_val;
	};
};

struct header {
	uint8_t type;
	uint32_t size;
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
	p->message = NULL;
	return p;
}

static void *Handle_print(void *arg)
{
	(void)arg;
	printf("We passed something through\n");
	return NULL;
}

static void *Handle_sendMirrorsMsg(Package *package)
{
	(void)package;
	// read lock
	// for mirror in mirrors
	//     establish connection
	//     send *message
	//     close socket
	// read unlock
	return NULL;
}

static void *Handle_sendMirrorAscii(Package *package)
{
	(void)package;
	// read lock
	// for mirror in mirrors
	//     establish connection
	//     send ascii_val
	//     close socket
	// read unlock
	return NULL;
}

static void Handle_revString(char *s)
{
	int end = strlen(s) - 1;
	for (int i = 0; i < end; ++i && --end) {
		s[i] = s[i] ^ s[end];
		s[end] = s[i] ^ s[end];
		s[i] = s[i] ^ s[end];
	}
}

void *Handle_request(void *arg)
{
	Package *package = arg;
	struct header pkt;

	if ((recv(package->rx_fd, &pkt, sizeof(pkt), 0)) == -1) {
		perror("recv");
		goto BAD_PKT;
	}
	if (pkt.type < 3 && pkt.size < USHRT_MAX) {
		package->message = calloc(pkt.size + 1, sizeof(char));
		switch(pkt.type) {
		case 0:
			if (send(package->rx_fd, package->message, pkt.size, 0) == -1) {
				perror("send_type0");
			}
			// queue task to send to mirrors, that thread will free
			// log event
			break;
		case 1:
			// function to invert string
			Handle_revString(package->message);
			if (send(package->rx_fd, package->message, pkt.size, 0) == -1) {
				perror("send_type1");
			}
			// same func() as case 0
			// log event
			break;
		case 2:
			//ascii_val = Handle_ascii(string); //will free
			//queue task to send to mirrors
			//log event
			break;
		}
	} else if (pkt.type == 3) {
		struct sockaddr_storage *addr = malloc(sizeof(*addr));
		socklen_t addr_sz = sizeof(struct sockaddr_storage);
		getpeername(package->rx_fd, (struct sockaddr *)addr, &addr_sz);
		Mirrors_add(package->mirrors, addr);
	}


	/*
		struct sockaddr_storage *addr = malloc(sizeof(*addr));
		socklen_t addr_sz = sizeof(struct sockaddr_storage);
		getpeername(package->rx_fd, (struct sockaddr *)addr, &addr_sz);
		Mirrors_add(package->mirrors, addr);
		Pool_addTask(package->pool, Handle_print, NULL);
	*/
BAD_PKT:
	close(package->rx_fd);
	free(package);
	return NULL;
}

// Beej networking guide
void *Handle_getAddr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// https://stackoverflow.com/questions/2371910/how-to-get-the-port-number-from-struct-addrinfo-in-unix-c
u_int16_t Handle_getPort(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return (((struct sockaddr_in *)sa)->sin_port);
	}
	return (((struct sockaddr_in6 *)sa)->sin6_port);
}
