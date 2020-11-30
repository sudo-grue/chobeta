#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "Handle.h"
#include "Pool.h"
#include "Socket.h"

#define PORT "3490"		// the port users will be connecting to
#define BACKLOG 10		// how many pending connections queue will hold

void sigint_handler(int s)
{
	(void)s;
}

int main(void)
{
	int sockfd, new_fd;	// listen on sock_fd, new connection on new_fd
	struct addrinfo *servinfo;
	struct sockaddr_storage their_addr;	// connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
//	char port[5] = "3490\0";

	struct sigaction si = {
		.sa_handler = sigint_handler,
	};

	if (sigaction(SIGINT, &si, NULL) == -1) {
		perror("sigint");
		exit(1);
	}
/*
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
*/
	servinfo = Socket_getServInfo(NULL, PORT);
	if (!servinfo) {
		exit(1);
	}
	sockfd = Socket_bindSocket(servinfo);

	freeaddrinfo(servinfo);

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	Pool *pool = Pool_create(152);
	if (!pool) {
		close(sockfd);
		exit(1);
	}
	Mirrors *mirrors = Mirrors_create();
	if (!mirrors) {
		close(sockfd);
		free(pool);
		exit(1);
	}

	printf("server: waiting for connections...\n");
	while (1) {
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			if (errno == EINTR) {
				printf("Shutting Down!\n");
				break;
			}
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, Handle_getAddr((struct sockaddr *)&their_addr), s, sizeof(s));
		printf("server: got connection from %s:%d\n", s, ntohs(Handle_getPort((struct sockaddr *)&their_addr)));

		Package *packet = Handle_createPackage(pool, mirrors, new_fd);
		if (!packet) {
			perror("createPackage");
			continue;
		}

		if (!Pool_addTask(pool, Handle_request, packet)) {
			perror("Pool_addTask");
			close(sockfd);
			close(new_fd);
			break;
		}

	}
	Pool_wait(pool);
	Mirrors_delete(mirrors);
	Pool_destroy(pool);
	close(sockfd);
	return 0;
}
