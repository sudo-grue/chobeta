#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "Handle.h"
#include "Pool.h"
#include <pthread.h>

#define PORT "3490"		// the port users will be connecting to
#define BACKLOG 10		// how many pending connections queue will hold

void sigint_handler(int s)
{
	(void)s;
}

int main(void)
{
	int sockfd, new_fd;	// listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;	// connector's address information
	socklen_t sin_size;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);	// all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	struct sigaction si = {
		.sa_handler = sigint_handler,
	};

	if (sigaction(SIGINT, &si, NULL) == -1) {
		perror("sigint");
		close(sockfd);
		exit(1);
	}

	Pool *pool = Pool_create(152);
	if (!pool) {
		close(sockfd);
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
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
		printf("server: got connection from %s:%d\n", s, ntohs(get_in_port((struct sockaddr *)&their_addr)));

		Package *packet = Handle_createPackage(pool, new_fd);
		if (!packet) {
			perror("createPackage");
			continue;
		}

		if (!Pool_addTask(pool, Handle_request, packet)) {
			perror("Pool_addTask");
			close(sockfd);
			close(new_fd);
			exit(1);
		}

	}
	Pool_wait(pool);
	Pool_destroy(pool);
	close(sockfd);
	return 0;
}
