#include "Mirrors.h"

#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>

struct mirror {
	Mirror *next;
	struct sockaddr_storage *sa;
};

struct mirrors {
	Mirror *head;
	pthread_rwlock_t lock;
};

Mirrors *Mirrors_create(void)
{
	Mirrors *m = malloc(sizeof(*m));
	if (!m) {
		return NULL;
	}

	m->head = NULL;
	pthread_rwlockattr_t rwlock_attr;
	pthread_rwlockattr_init(&rwlock_attr);
	pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
	pthread_rwlock_init(&m->lock, &rwlock_attr);

	return m;
}

bool Mirrors_add(Mirrors *m, struct sockaddr_storage *sa)
{
	if (!m || !sa) {
		return false;
	}

	Mirror *mirror = malloc(sizeof(*mirror));
	if (!mirror) {
		return false;
	}

	mirror->sa = sa;
	pthread_rwlock_wrlock(&m->lock);
	{
		mirror->next = m->head;
		m->head = mirror;
	}
	pthread_rwlock_unlock(&m->lock);

	return true;
}

void Mirrors_rm(Mirrors *m, struct sockaddr_storage *sa)
{
	if (!m || !sa) {
		return;
	}
	Mirror *curr = m->head;
	Mirror *next = m->head->next;
	if (curr->sa == sa) {
		pthread_rwlock_wrlock(&m->lock);
		{
			// verify it's still there after lock
			if (curr->sa == sa) {
				m->head = next;
			}
		}
		pthread_rwlock_unlock(&m->lock);
		curr->next = NULL;
		free(curr->sa);
		free(curr);
	} else {
		while (next) {
			if (next->sa == sa) {
				pthread_rwlock_wrlock(&m->lock);
				{
					if (next->sa == sa) {
						curr->next = next->next;
					}
				}
				pthread_rwlock_unlock(&m->lock);
				curr->next = NULL;
				free(curr->sa);
				free(curr);
			}
			curr = next;
			next = next->next;
		}
	}
}

void Mirrors_delete(Mirrors *m)
{
	if (!m) {
		return;
	}
	Mirror *temp = m->head;
	pthread_rwlock_wrlock(&m->lock);
	while (temp) {
		Mirror *prev = temp;
		temp = temp->next;
		prev->next = NULL;
		free(prev->sa);
		free(prev);
	}
	pthread_rwlock_unlock(&m->lock);
	pthread_rwlock_destroy(&m->lock);
	free(m);
}
