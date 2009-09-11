#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

struct task_pool; 

struct task_pool *task_pool_init(size_t max);
int task_pool_free(struct task_pool *poolh);

int task_pool_add(struct task_pool *poolh, void *(*task_func)(void *arg), void *data);
/*int task_pool_status(struct task_pool *poolh) */

#endif

