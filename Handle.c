#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
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

static void *Handle_sendMirrorsMsg(void *arg);
static void *Handle_sendMirrorsAscii(void *arg);
static void Handle_revString(char *s);
static void Handle_ascii(Package *p);

// main() for threads
void *Handle_request(void *arg)
{
	Package *pkg = arg;
	struct header pkt;
	bool pkg_unused = true;


	if ((recv(pkg->rx_fd, &pkt, sizeof(pkt), 0)) == -1) {
		perror("recv");
		close(pkg->rx_fd);
		free(pkg);
		return NULL;
	}
	if (pkt.type < 3 && pkt.size < USHRT_MAX) {
		pkg->message = calloc(pkt.size + 1, sizeof(char));
		//log event
		pkg_unused = false;
		switch(pkt.type) {
		case 0:
			if (send(pkg->rx_fd, pkg->message, pkt.size, 0) == -1) {
				perror("send_type0");
			}
			Pool_addTask(pkg->pool, Handle_sendMirrorsMsg, pkg);
			// queue task to send to mirrors, that thread will free
			break;
		case 1:
			Handle_revString(pkg->message);
			if (send(pkg->rx_fd, pkg->message, pkt.size, 0) == -1) {
				perror("send_type1");
			}
			Pool_addTask(pkg->pool, Handle_sendMirrorsMsg, pkg);
			// same func() as case 0
			break;
		case 2:
			Handle_ascii(pkg);
			if (send(pkg->rx_fd, &pkg->ascii_val, sizeof(pkg->ascii_val), 0) == -1) {
				perror("send_type2");
			}
			Pool_addTask(pkg->pool, Handle_sendMirrorsAscii, pkg);
			//queue task to send to mirrors
			break;
		}
	} else if (pkt.type == 3 && pkt.size == 0) {
		struct sockaddr_storage *addr = malloc(sizeof(*addr));
		socklen_t addr_sz = sizeof(struct sockaddr_storage);
		getpeername(pkg->rx_fd, (struct sockaddr *)addr, &addr_sz);
		Mirrors_add(pkg->mirrors, addr);
	}

	close(pkg->rx_fd);
	if (pkg_unused) {
		free(pkg);
	}
	return NULL;
}

Package *Handle_createPackage(Pool *pool, Mirrors *mirrors, int rx_fd)
{
	Package *pkg = malloc(sizeof(*pkg));
	if (!pkg) {
		return NULL;
	}
	pkg->pool = pool;
	pkg->mirrors = mirrors;
	pkg->rx_fd = rx_fd;
	pkg->message = NULL;
	return pkg;
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

static void *Handle_sendMirrorsMsg(void *arg)
{
	Package *pkg = arg;
	Mirrors *m = pkg->mirrors;
	pthread_rwlock_rdlock(&m->lock);
	{
	// for mirror in mirrors
	//     establish connection
	//     send *message
	//     close socket
	}
	pthread_rwlock_unlock(&m->lock);
	free(pkg->message);
	free(pkg);
	return NULL;
}

static void *Handle_sendMirrorsAscii(void *arg)
{
	Package *pkg = arg;
	Mirrors *m = pkg->mirrors;
	pthread_rwlock_rdlock(&m->lock);
	{
	// for mirror in mirrors
	//     establish connection
	//     send ascii_val
	//     close socket
	}
	pthread_rwlock_unlock(&m->lock);
	free(pkg);
	return NULL;
}

static void Handle_revString(char *str)
{
	int end = strlen(str) - 1;
	for (int i = 0; i < end; ++i) {
		str[i] = str[i] ^ str[end];
		str[end] = str[i] ^ str[end];
		str[i] = str[i] ^ str[end];
		--end;
	}
}

static void Handle_ascii(Package *pkg)
{
	uint64_t ret = 0;
	char *str = pkg->message;

	while (*str != '\0') {
		ret += *str;
		++str;
	}
	free(pkg->message);
	pkg->ascii_val = ret;
}

