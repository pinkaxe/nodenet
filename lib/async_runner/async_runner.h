#ifndef __ASYNC_RUNNER
#define __ASYNC_RUNNER

struct async_runner;

struct async_runner *async_runner_init(int num);
int async_runner_free(struct async_runner *h);
int async_runner_exec(struct async_runner *h, void *(*func)(void *arg),
    void *data, void (*res_func)(void *arg, bool succ));

#endif
