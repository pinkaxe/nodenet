#ifndef __ASYNC_RUNNER
#define __ASYNC_RUNNER

struct async_runner;

struct async_runner *async_runner_run(int num);
int async_runner *async_runner_free(struct thread_pool *poolh);
int async_runner_exec(struct thread_pool *poolh, void *(*func)(void *arg),
    void *data, void (*res_func)(void *arg, bool succ));

#endif
