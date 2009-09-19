
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "arch/thread.h"
#include "util/log.h"
#include "util/dpool.h"
#include "util/que.h"
#include "util/ll.h"

#include "code_net/code.h"

#define MAX_LINKS   4

struct code_link {
    struct code_elem *link;
    struct link ll_link;
};

struct code_elem
{
    code_type_t type; /* thread, process etc. */
    code_attr_t attr;
    void *code;  /* pointer to object depending on type */

    //code_elem_t *links[MAX_LINKS]; /* links to other code_elem's */
    
    struct ll *linksh;
    struct que *in_bufs;         /* input for code elem */
    //struct ll *in_bufs_cbs;         /* action on in_bufs callbacks */
    struct que *cmd_bufs;        /* input cmds for code elem */
    //struct ll *cmd_bufs_cbs;     /* action on cmd_bufs callbacks */

    struct ll *out_bufs_cb;        /* action on out_bufs callbacks */

    void *pdata; /* private passthru data */
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

    h->cmd_bufs = que_init(8);
    if(!h->cmd_bufs){
        printf("goto err\n");
        goto err;
    }

    int err;
    h->linksh = ll_init(struct code_link, ll_link, &err);
    if(!h->linksh){
        printf("goto err\n");
        goto err;
    }

err:
    return h;
}

//code_free();

int code_link(code_elem_t *from, code_elem_t *to)
{
    int i;
    struct code_link *start, *curr, *track;

    struct code_link *link;

    link = malloc(sizeof(*link));
    if(!link){
        printf("cunt\n");
        exit(-1);
        //LOG1(
    }

    link->link = to;
    ll_add_front(from->linksh, link);

   // start = ll_get_start(from->linksh);
   // ll_foreach(start, curr, track){
   //     if(curr-> 
   // }
   // //ll_foreach(start, curr, track){
   // for(i=0; i < MAX_LINKS; i++){
   //     if(!from->links[i]){
   //         from->links[i] = to;
   //         break;
   //     }
   // }
   // if(i == MAX_LINKS){
   // }

    return 0;
}

int code_unlink(code_elem_t *from, code_elem_t *to)
{
}

struct code_cmd {
    int id;
    void *data;
};

int code_tx_cmd(code_elem_t *to, int id, void *data)
{
    struct code_cmd *cmd = malloc(sizeof(*cmd));
    cmd->id = id;
    cmd->data = data;

    return que_add(to->cmd_bufs, cmd);
}

int code_tx_data(code_elem_t *to, void *buf, int len)
{
    return que_add(to->in_bufs, buf);
}

int code_out_avail(code_elem_t *e, buf_attr_t attr, void *buf, int len, void
        (*sending_to_no_cb)(void *buf, int no))
{
    int i, c;
    code_elem_t *to;

    // count first
   // for(c=0,i=0; i < MAX_LINKS; i++){
   //     if((e->links[i])){
   //         c++;
   //     }
   // }
    struct code_link *curr; 
    struct code_link *track = NULL;
    struct code_link *start = ll_get_start(e->linksh); 
    printf("zz %p\n", start);

    // add to datastructure
    ll_foreach(e->linksh, start, curr, track){
        code_tx_data(curr->link, buf, len);
    }

    //if(c > 0){
    //    /* notify upstairs */
    //    sending_to_no_cb(buf, c);
    //    for(i=0; i < MAX_LINKS; i++){
    //        /* signal all */
    //        printf("loop %p\n", e->links[i]);

    //        if((to=e->links[i])){
    //            code_tx_data(to, buf, 1);
    //        }
    //    }
    //}
}


static void *code_run_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    code_elem_t *h = arg;
    void (*func)(code_elem_t *h, void *buf, int len, void *pdata) = h->code;
    struct timespec buf_check_timespec;
    struct timespec cmd_check_timespec;

    for(;;){

        buf_check_timespec.tv_sec = 2;
        buf_check_timespec.tv_nsec = 10000000;
        cmd_check_timespec.tv_sec = 2;
        cmd_check_timespec.tv_nsec = 10000000;

        if((h->attr & CODE_ATTR_NO_INPUT)){
            /* call user function */
            func(h, NULL, 0, h->pdata);

        }else{
            /* incoming data */
            buf = que_get(h->in_bufs, &buf_check_timespec);
            if(buf){
                /* call user function */
                func(h, buf, 1, h->pdata);
            }else{
                if(errno == ETIMEDOUT){
                    //printf("!! timedout xx\n");
                }
            }
            //sleep(1);
        }

        /* incoming commands */
        cmd_buf = que_get(h->cmd_bufs, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\n ");
            free(cmd_buf);
            break;
            // process and free
        }else{
            if(errno == ETIMEDOUT){
                //printf("!! timedout xx\n");
            }
        }

    }

    printf("... thread exit\n");
    return NULL;
}

static void *code_run_bin(void *arg)
{
    code_elem_t *h = arg;
    char *filename = h->code;

    for(;;){
        //code_wait(h);
        // exe filename giving input buffer as input
    }

    return NULL;
}

static void *code_run_net(void *arg)
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


