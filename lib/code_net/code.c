
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "arch/thread.h"
#include "util/dpool.h"
#include "util/log.h"
#include "util/que.h"

#include "code_net/code.h"

#define MAX_LINKS   4

struct comm_buf {
    void *buf;
    int len;
    int ref_count;
};

struct code_elem
{
    code_type_t type; /* thread, process etc. */
    code_attr_t attr;
    void *code;  /* pointer to object depending on type */

    code_elem_t *links[MAX_LINKS]; /* links to other code_elem's */
    struct que *in_bufs;         /* place to add and get bufs */
    struct que *cmd_bufs;        /* place to add and get cmds */

    void *pdata; /* private passthru data*/
};


code_elem_t *code_create(code_type_t type, code_attr_t attr, void *code,
        void *pdata)
{
    printf("crete\n");
    code_elem_t *h;

    h = malloc(sizeof(*h));
    if(!h){
        goto err;
    }

    h->type = type;
    h->attr = attr;
    h->code = code;
    h->pdata = pdata;

    h->in_bufs = que_init(8);
    if(!h->in_bufs){
        printf("goto err\n");
        goto err;
    }

    memset(h->links, 0x00, sizeof(h->links));

err:
    return h;
}

//code_free();

int code_link(code_elem_t *from, code_elem_t *to)
{
    int i;

    for(i=0; i < MAX_LINKS; i++){
        if(!from->links[i]){
            from->links[i] = to;
            break;
        }
    }
    if(i == MAX_LINKS){
    }

    return 0;
}

int code_unlink(code_elem_t *from, code_elem_t *to)
{
}

int code_out_avail(code_elem_t *e, buf_attr_t attr, void *buf, int len, void
        (*sending_to_no_cb)(void *buf, int no))
{
    int i, c;
    code_elem_t *to;

    // count first
    for(c=0,i=0; i < MAX_LINKS; i++){
        if((e->links[i])){
            c++;
        }
    }

    if(c > 0){
        /* notify upstairs */
        sending_to_no_cb(buf, c);
        for(i=0; i < MAX_LINKS; i++){
            /* signal all */
            printf("loop %p\n", e->links[i]);

            if((to=e->links[i])){
                que_add(to->in_bufs, buf);
                printf("send sig\n");
            }
        }
    }
}


void *code_run_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    code_elem_t *h = arg;
    void (*func)(code_elem_t *h, void *buf, int len, void *pdata) = h->code;

    for(;;){

        if((h->attr & CODE_ATTR_NO_INPUT)){
            /* call user function */
            func(h, NULL, 0, h->pdata);

        }else{
            buf = que_get(h->in_bufs, 100);
            if(buf){
                /* call user function */
                func(h, buf, 1, h->pdata);
            }
        }

        //cmd_buf = que_get(h->cmd_bufs, 100);
        //if(cmd_buf){
        //}

    }

    return NULL;
}

void *code_run_bin(void *arg)
{
    code_elem_t *h = arg;
    char *filename = h->code;

    for(;;){
        //code_wait(h);
        // exe filename giving input buffer as input
    }

    return NULL;
}

void *code_run_net(void *arg)
{
    // setup control channel
    //
    for(;;){
        //code_wait(h);
        // serialize buffer and send
    }
}

int code_run(code_elem_t *h)
{
    if(h->type == CODE_TYPE_THREAD){
        thread_t tid;
        thread_create(&tid, NULL, code_run_thread, h);
    }else if(h->type == CODE_TYPE_BIN){
        thread_t tid;
        thread_create(&tid, NULL, code_run_bin, h);
    }
}

int code_end(code_elem_t *h)
{
}

/*

int code_net_set_ev_cb()
{
}


code_net_walk(struct code_net *root)
{
    //struct code *code;

    for(;;){
        code_run();
    }
}

*/


