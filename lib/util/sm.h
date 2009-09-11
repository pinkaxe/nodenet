
#ifndef __SM_H__
#define __SM_H__

struct sm;

typedef int (*sm_event_cb)(int e, void *data);

struct sm_func {
    int state;
    int (*func)(struct sm *h, void *pdata, sm_event_cb cb);
    int next_state;
};

struct sm *sm_init(struct sm_func *sm_func, int sm_func_no, int *err);
void sm_free(struct sm *h);


int sm_run(struct sm *h, size_t c, void *pdata, sm_event_cb cb);
 /*(struct state_func *state_func, int state_func_no, struct sm_state *s, sm_event_cb cb);*/
int sm_loop(struct sm *h);


int sm_set_next_state(struct sm *h, int state);

#endif
