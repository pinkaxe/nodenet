
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "arch/thread.h"
#include "util/dpool.h"
#include "util/log.h"
#include "util/cybuf.h"

#include "code_net/code.h"

#define MAX_LINKS   4

/*
void *code_t_thread_thread(void *arg)
{
    // Get stuff from arg
    func(in_buf, in_buf_len, out_buf, out_buf_len);
}
*/

typedef struct code_elem {

    code_t type;
    void *code;
    void *pdata;

    mutex_t mutex;
    cond_t cond;

    unsigned int out_code_no;
    struct code_elem *links[MAX_LINKS];

    int signals_in;

    struct cybuf *in_bufs;

} code_elem_t;

typedef struct code_net {
    struct code_elem *root;
} code_net_t;



code_elem_t *code_create(code_t type, void *code, void *pdata)
{
    printf("crete\n");
    code_elem_t *h;

    h = malloc(sizeof(*h));
    if(!h){
        goto err;
    }

    h->type = type;
    h->code = code;
    h->pdata = pdata;

    memset(h->links, 0x00, sizeof(h->links));

    h->signals_in = 0;
    h->in_bufs = cybuf_init(8);
    //if(!h->in_bufs){

    mutex_init(&h->mutex, NULL);
    cond_init(&h->cond, NULL);

err:
    return h;
}

int code_link(struct code_elem *e0, struct code_elem *e1)
{
    int i;

    for(i=0; i < MAX_LINKS; i++){
        if(!e0->links[i]){
            e0->links[i] = e1;
            break;
        }
    }
    if(i == MAX_LINKS){
    }

    return 0;
}

int code_unlink(struct code_elem *e0, struct code_elem *e1)
{
}

int code_out_avail(struct code_elem *e, int type, void *buf, int len) // TYPE = all, one, specific ones
{
    int i;
    struct code_elem *signal;

    for(i=0; i < MAX_LINKS; i++){
        /* signal all */
        printf("loop %p\n", e->links[i]);

        if((signal=e->links[i])){
            signal->signals_in++;
            cybuf_add(signal->in_bufs, buf);
            printf("send sig\n");
            // threads
            cond_signal(&signal->cond);
            // process
            // send signal on control channel
        }
    }
}

int code_wait(struct code_elem *e)
{
    while(e->signals_in < 1){
        cond_wait(&e->cond, &e->mutex);
    }
    e->signals_in--;
    printf("Got sig\n");
}

void *run_thread(void *arg)
{
    void *buf = NULL;
    code_elem_t *h = arg;
    void (*func)(code_elem_t *h, void *buf, int len, void *pdata) = h->code;

    for(;;){
        if(!(h->type & CODE_NO_INPUT)){
            code_wait(h);
            buf = cybuf_get(h->in_bufs);
        }

        // pass buffer
        func(h, buf, 1, h->pdata);
    }

    return NULL;
}

void *run_bin(void *arg)
{
    code_elem_t *h = arg;
    char *filename = h->code;

    for(;;){
        code_wait(h);
        // exe filename giving input buffer as input
    }

    return NULL;
}

void *run_net(void *arg)
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
    if(h->type & code_t_thread){
        thread_t tid;
        thread_create(&tid, NULL, run_thread, h);
    }else if(h->type & code_t_bin){
        thread_t tid;
        thread_create(&tid, NULL, run_bin, h);
    }
}


/*
code_free();
code_link(net, in, out, int out_buf_size, )
code_unlink(net, in, out, int out_buf_size, )


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


