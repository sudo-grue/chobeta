#ifndef SOCKET_H
#define SOCKET_H

#include <netdb.h>

struct addrinfo *Socket_getServInfo(const char *host, const char *port);
int Socket_bindSocket(struct addrinfo *servinfo);
//int Socket_netInfo(char *node, char *port, struct addrinfo *hints, struct addrinfo **res);

#endif
