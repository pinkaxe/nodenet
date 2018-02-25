#ifndef __THREAD_POOL
#define __THREAD_POOL

struct thread_pool; 

struct thread_pool *thread_pool_init(int num, void *(*thread_func)(void *arg));
int thread_pool_free(struct thread_pool *poolh);
int thread_pool_add_work(struct thread_pool *poolh, void *data,
	void (*res_func)(void *arg, bool succ));

#endif
