#include "Queue.h"

#include <pthread.h>
#include <stdlib.h>

struct llist {
	struct llist *next;
	Task t;
	void *arg;
};

struct queue {
	struct llist *head;
	struct llist *tail;
	pthread_mutex_t lock;
};

Queue *Queue_create(void)
{
	Queue *q = malloc(sizeof(*q));
	if (!q) {
		return NULL;
	}

	q->head = q->tail = NULL;
	pthread_mutex_init(&q->lock, NULL);

	return q;
}

bool Queue_enqueue(Queue *q, Task t, void *arg)
{
	if (!q || !t) {
		return false;
	}

	struct llist *new = malloc(sizeof(*new));
	if (!new) {
		return false;
	}

	new->t = t;
	new->arg = arg;
	new->next = NULL;

	pthread_mutex_lock(&q->lock);
	{
		if (q->tail) {
			q->tail->next = new;
		} else {
			q->head = new;
		}

		q->tail = new;
	}
	pthread_mutex_unlock(&q->lock);

	return true;
}

void Queue_dequeue(Queue *q, Task *t, void **arg)
{
	if (!q || !t || !arg) {
		return;
	}

	pthread_mutex_lock(&q->lock);
	{
		if (!q->head) {
			*t = NULL;
			*arg = NULL;
			goto unlock;
		}

		*t = q->head->t;
		*arg = q->head->arg;

		struct llist *tmp = q->head;

		q->head = q->head->next;
		if (!q->head) {
			q->tail = NULL;
		}

		free(tmp);
	}
 unlock:
	pthread_mutex_unlock(&q->lock);
}

void Queue_destroy(Queue *q)
{
	if (!q) {
		return;
	}

	struct llist *curr = q->head;
	while (curr) {
		struct llist *tmp = curr;
		curr = curr->next;
		free(tmp);
	}

	free(q);
}
