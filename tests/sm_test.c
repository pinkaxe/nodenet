

#include<stdio.h>

#include "util/sm.h"


#if 0
enum states {
    INPUT = 0x01,
    PROCESS = 0x02
};

static int start(void *pdata, sm_event_cb cb);
static int end(void *pdata, sm_event_cb cb);
static int input(void *pdata, sm_event_cb cb);
static int process(void *pdata, sm_event_cb cb);

static struct sm_func _state_func[] = {
    {INPUT,     input},
    {PROCESS,   process},
};
static int _state_func_no = sizeof(_state_func) / sizeof(_state_func[0]);


static int start(void *pdata, sm_event_cb event_cb)
{
    /* notify upstairs */
    event_cb(0, NULL);
    return 0;
}

static int input(void *pdata, sm_event_cb event_cb)
{
    event_cb(1, NULL);
    return 0;
}

static int process(void *pdata, sm_event_cb event_cb)
{
    event_cb(2, NULL);
    return 0;
}

static int end(void *pdata, sm_event_cb event_cb)
{
    event_cb(3, NULL);
    return 0;
}



int event_cb(int e, void *data)
{
    printf("event cb: %d\n", e);
    return 0;
}


int main(int argc, char **argv)
{
    int r;
    int err;
    struct sm *smh;


    for(;;){
        smh = sm_init(_state_func, _state_func_no, &err);
        if(!smh){
            printf("err: %d\n", err);
            break;
        }

        while(1){
            r = sm_run(smh, 1, NULL, event_cb);
        }

        sm_free(smh);
    }
    end();

    return 0;
}
#endif

int main(int argc, char **argv)
{
}
