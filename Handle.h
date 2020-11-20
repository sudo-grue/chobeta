#ifndef HANDLE_H
#define HANDLE_H

#include <stdlib.h>
#include "Pool.h"

typedef struct package Package;

Package *Handle_createPackage(Pool *p, int rx_fd);
void *Handle_request(void *arg);
void *get_in_addr(struct sockaddr *sa);
u_int16_t get_in_port(struct sockaddr *sa);

#endif
