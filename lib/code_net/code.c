
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

struct code_elem
{
    code_type_t type;
    code_attr_t attr;
    void *code;  /* pointer to object depending on type */

    code_elem_t *links[MAX_LINKS]; /* links to other code_elem's */
    struct que *in_bufs;         /* place to add and get bufs */

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

int code_out_avail(code_elem_t *e, int type, void *buf, int len)
{
    int i;
    code_elem_t *to;

    for(i=0; i < MAX_LINKS; i++){
        /* signal all */
        printf("loop %p\n", e->links[i]);

        if((to=e->links[i])){
            que_add(to->in_bufs, buf);
            printf("send sig\n");
        }
    }
}

void *code_run_thread(void *arg)
{
    void *buf = NULL;
    code_elem_t *h = arg;
    void (*func)(code_elem_t *h, void *buf, int len, void *pdata) = h->code;

    for(;;){
        if(!(h->attr & CODE_ATTR_NO_INPUT)){
            buf = que_get(h->in_bufs, 0);
        }

        /* call user function */
        func(h, buf, 1, h->pdata);
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


// app 

void *in_code(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int c;

    for(;;){
        c = getc(stdin);

        b = get_free_out_buf();
        b[0] = c;
        signal;
    }

}

void *code_upper(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    for(;;){
        wait_for_input();

        b = get_free_out_buf();

        b[0] = toupper(in_buf[0]);
        get_free_ret_buf();
        fill;
        signal;

    }

}

void *out_code(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    for(;;){
        wait_for_input();
        printf;
    }
}


int main(int argc, char const* argv[])
{
    struct code_net;
    struct code *e0, *e1, *e2;

    e0 = code_create(0, 128, code_t_thread, in_code);

    //e1 = code_create(128, 128, code_t_shell, "test.sh");
    //code_conn(code_net, e0, e1);
    e1 = code_create(128, 128, code_t_thread, code_upper);
    code_link(code_net, e0, e1, buf0, link0_cb);

    e2 = code_create(128, 0, code_t_thread, out_code);
    cl1 = code_link(code_net, e1, e2, buf1, link1_cb);

    link_snoop(cl1)

    code_net_set_ev_cb();

    code_net_run();

    return 0;
}


*/


