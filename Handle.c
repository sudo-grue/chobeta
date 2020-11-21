#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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
	char *message;
};

struct header {
	uint8_t type;
	uint32_t size;
};

static void *Handle_sendMirrorsMsg(void *arg);
static void Handle_revString(char *s);
static void Handle_ascii(Package *pkg);

// main() for thread handling inbound socket connections
void *Handle_request(void *arg)
{
	Package *pkg = arg;
	struct header pkt;
	bool pkg_unused = true;

	if ((recv(pkg->rx_fd, &pkt, sizeof(pkt), 0)) == -1) {
		perror("recv");
		pkt.type = 4;
	}
	if (pkt.type < 3 && pkt.size < USHRT_MAX) {
		pkg->message = calloc(pkt.size + 1, sizeof(pkg->message));
		if (!pkg->message) {
			perror("alloc()");
			goto CLOSE;
		}

		if ((recv(pkg->rx_fd, pkg->message, pkt.size, 0)) == -1) {
			perror("recv2");
			goto CLOSE;
		}
		printf("[%d, %d] - %s\n", pkt.type, pkt.size, pkg->message);
		//log event

		pkg_unused = false;
		switch(pkt.type) {
		case 1:
			Handle_revString(pkg->message);
			break;
		case 2:
			Handle_ascii(pkg);
			pkt.size = strlen(pkg->message);
			break;
		}
		if (send(pkg->rx_fd, pkg->message, pkt.size, 0) == -1) {
			perror("send_type0");
		}
		//TODO: Remove these free's after sendMirrorMsg written
		Pool_addTask(pkg->pool, Handle_sendMirrorsMsg, pkg); //must free

	} else if (pkt.type == 3 && pkt.size == 0) {
		struct sockaddr_storage *addr = malloc(sizeof(*addr));
		socklen_t addr_sz = sizeof(struct sockaddr_storage);
		getpeername(pkg->rx_fd, (struct sockaddr *)addr, &addr_sz);
		Mirrors_add(pkg->mirrors, addr);
	}
CLOSE:
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

//TODO: Validation and error handling
static void *Handle_sendMirrorsMsg(void *arg)
{
	Package *pkg = arg;
	Mirrors *m = pkg->mirrors;
	struct sockaddr_storage their_addr;
	struct addrinfo hints, *servinfo, *p;
	int rv, tx_fd;
	u_int16_t port;
	char s[INET6_ADDRSTRLEN];
	char port_str[8];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	pthread_rwlock_rdlock(&m->lock);
	{
		Mirror *mir = m->head;
		while (mir) {
			their_addr = *mir->sa;
			// populate s with distant end
			inet_ntop(their_addr.ss_family, Handle_getAddr((struct sockaddr *)&their_addr), s, sizeof(s));
			// populate port_str with distant end
			port = ntohs(Handle_getPort((struct sockaddr *)&their_addr));
			sprintf(port_str, "%d", port);
			
			if ((rv = getaddrinfo(s, port_str, &hints, &servinfo)) != 0) {
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				break;
			}
			printf("Trying %s:%s\n", s, port_str);
			for(p = servinfo; p != NULL; p = p->ai_next) {
				if ((tx_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}

				if (connect(tx_fd, p->ai_addr, p->ai_addrlen) == -1) {
					close(tx_fd);
					perror("client: connect");
					continue;
				}
				break;
			}
			if (p == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				return NULL;
			}
			freeaddrinfo(servinfo);
			if (send(tx_fd, pkg->message, strlen(pkg->message), 0) == -1) {
				perror("send");
			}
			close(tx_fd);
			mir = mir->next;
		}
	}
	pthread_rwlock_unlock(&m->lock);
	free(pkg->message);
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

	int num_char = 0;
	while (*str != '\0') {
		ret += *str;
		++str;
		++num_char;
	}
	if (num_char < 5) {
		pkg->message = realloc(pkg->message, 8);
		num_char = 7;
	}
	snprintf(pkg->message, num_char, "%ld", ret);
}

