#ifndef POOL_H
#define POOL_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct pool Pool;

typedef void *(*Task) (void *);

/**
 * Create a new worker pool consisting of numWorkers
 *
 * @param number of workers in pool
 * @return new worker pool
 */
Pool *Pool_create(size_t numWorkers);

/**
 * Add a task to the pool
 *
 * @param pool to add task to
 * @param task to add
 * @param argument to task
 * @return true if successfully added, false otherwise
 */
bool Pool_addTask(Pool *p, Task t, void *arg);

/**
 * Waits for all tasks in pool to be executed
 *
 * @param pool to drain
 */
void Pool_wait(Pool *p);

/**
 * Free up resources from pool
 * DOES NOT WAIT for workers to finish tasks
 *
 * @param pool to destroy
 */
void Pool_destroy(Pool *p);

#endif
