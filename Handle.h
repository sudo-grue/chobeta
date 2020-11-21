#ifndef HANDLE_H
#define HANDLE_H

#include <stdlib.h>
#include "Pool.h"
#include "Mirrors.h"

typedef struct package Package;

Package *Handle_createPackage(Pool *p, Mirrors *m, int rx_fd);
void *Handle_request(void *arg);
void *Handle_getAddr(struct sockaddr *sa);
u_int16_t Handle_getPort(struct sockaddr *sa);

#endif
