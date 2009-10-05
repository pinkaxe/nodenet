
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include "sm.h"


struct sm {
    struct sm_func *sm_func;
    int sm_func_no;
    int state_curr;
    sm_event_cb cb;
    void *pdata;
};

/*
int sm_state_add(int state, func func);
int sm_state_rem(int state);
*/

struct sm *sm_init(struct sm_func *sm_func, int sm_func_no, int *err)
{
    struct sm *sm;

    sm = malloc(sizeof(struct sm));
    if(!sm){
        *err = errno;
        goto err;
    }

    sm->sm_func = sm_func;
    sm->sm_func_no = sm_func_no;

err:
    return sm;
}


void sm_free(struct sm *h)
{
    free(h);
}


int sm_run(struct sm *h, size_t c, void *pdata, sm_event_cb cb)
{
    int ret = 0;
    int i;

    while(c--){
        for(i=0; i < h->sm_func_no; i++){
            printf("i=%d\n", i);
            h->state_curr = h->sm_func[i].state;
            ret = h->sm_func[i].func(h, pdata, cb);
            if(ret < 0){
                printf("error\n");
            }
        }
    }
    
    return ret;
}

#if 0
int sm_loop(struct sm *h)
{
    int ret;

    ret = sm_exe(h);
    while(h->state_curr != h->state_end){
        ret = sm_exe(h);
    }

        /*
    while(h->state_curr != h->state_end)
        ret = sm_exe(h);
        */

    /* one more time to execute state_end */
    ret = sm_exe(h);

    return ret;
}
#endif


int sm_set_next_state(struct sm *h, int state)
{
    h->state_curr = state;
    return 0;
}

int sm_get_state(struct sm *h)
{
    return h->state_curr;
}

