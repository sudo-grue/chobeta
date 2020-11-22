#ifndef MIRRORS_H
#define MIRRORS_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef struct mirror {
	struct mirror *next;
	struct sockaddr_storage *sa;
} Mirror;

typedef struct mirrors {
	Mirror *head;
	pthread_rwlock_t lock;
} Mirrors;

Mirrors *Mirrors_create(void);
bool Mirrors_add(Mirrors *m, struct sockaddr_storage *sa);
void Mirrors_rm(Mirrors *m, Mirror *del);
void Mirrors_delete(Mirrors *m);

#endif
