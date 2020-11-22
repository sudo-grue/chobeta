#include "Mirrors.h"

#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>

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

void Mirrors_rm(Mirrors *m, Mirror *del)
{
	if (!m || !del) {
		return;
	}
	Mirror *curr = m->head;
	Mirror *next = m->head->next;

	pthread_rwlock_wrlock(&m->lock);
	{
		if (curr == del) {
			m->head = next;
			curr->next = NULL;
			free(curr->sa);
			free(curr);
		} else {
			while (next) {
				if (next == del) {
					curr->next = next->next;
					free(next->sa);
					free(next);
					break;
				}
				curr = next;
				next = next->next;
			}
		}
	}
	pthread_rwlock_unlock(&m->lock);
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
