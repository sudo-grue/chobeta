#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Socket.h"

struct addrinfo *Socket_getServInfo(const char *host, const char *port)
{
	int rv;
	struct addrinfo hints, *servinfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
	}
	return servinfo;
}

int Socket_bindSocket(struct addrinfo *servinfo)
{
        struct addrinfo *p;
        int sockfd;
        bool yes = 1;
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
        if (p == NULL) {
                fprintf(stderr, "server: failed to bind\n");
                exit(1);
        }
        return sockfd;
}
