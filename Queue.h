#ifndef QUEUE_H
#define QUEUE_H

// For type Task
#include "Pool.h"

typedef struct queue Queue;

/**
 * Create a new queue
 *
 * @return new queue
 */
Queue *Queue_create(void);

/**
 * Add a task to the queue
 *
 * @param queue to enqueue to
 * @param task to add
 * @param argument to task
 * @return true if successfully added, false otherwise
 */
bool Queue_enqueue(Queue * q, Task t, void *arg);

/**
 * Remove a task from the queue
 *
 * @param queue to dequeue from
 * @param task from queue (output parameter)
 * @param argument for task from queue (output parameter)
 */
void Queue_dequeue(Queue * q, Task * t, void **arg);

/**
 * Free up resources from queue
 *
 * @param queue to destroy
 */
void Queue_destroy(Queue * q);

#endif
