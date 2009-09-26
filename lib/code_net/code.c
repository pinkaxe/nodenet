
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
#include "code_net/code_grp.h"

#define MAX_LINKS   4

struct code_net_elem {
    struct code_elem *elem;
    struct link link;
};

struct code_net {
    struct code_net_elem *elem;
};


struct code_link {
    struct code_elem *link;
    struct link ll_link;
    // quality etc., specific for link
};

struct code_data_req{
    struct code_elem *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

struct code_ctrl_req{
    struct code_elem *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
};

struct code_elem {
    code_type_t type; /* thread, process etc. */
    code_attr_t attr;
    void *code;  /* pointer to object depending on type */

    /* pointers for relationships */
    struct ll *out_links_llh;         /* links to other code_elem's */
    struct ll *in_links_llh;          /* links coming in from other code_elem's */
    struct ll *group_llh;             /* links to all groups this elem belongs to */
    struct ll *net_llh;               /* links to all groups this elem belongs to */

    /* bufs */
    struct que *in_data_queh;         /* input for code elem */
    struct que *in_cmd_queh;          /* input cmds for code elem */

    struct que *out_data_queh;        /* output to other code elem's */
    struct que *out_cmd_queh;         /* input cmds for code elem */

    void *pdata; /* private passthru data */
};



code_elem_t *code_create(code_type_t type, code_attr_t attr, void *code,
        void *pdata)
{
    int err;
    code_elem_t *h;

    PCHK(LWARN, h, malloc(sizeof(*h)));
    if(!h){
        goto err;
    }

    PCHK(LWARN, h->in_data_queh = que_init(8));
    if(!h->in_data_queh){
        goto err;
    }

    PCHK(LWARN, h->in_cmd_queh = que_init(8));
    if(!h->in_cmd_queh){
        goto err;
    }

    PCHK(LWARN, h->linksh = ll_init(struct code_link, ll_link, &err));
    if(!h->linksh){
        goto err;
    }

    h->type = type;
    h->attr = attr;
    h->code = code;
    h->pdata = pdata;

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
    ll_add_front(from->out_linksh, link);

    // link back
    link = malloc(sizeof(*link));
    if(!link){
        printf("cunt\n");
        exit(-1);
        //LOG1(
    }

    ll_add_front(to->in_linksh, link);

    return 0;
}

struct code_net *code_net_init()
{
    struct code_net *h;

    return h;
}

void code_net_free(struct code_net *h)
{
}

int code_net_add_memb(struct code_net *h, code_elem_t *to)
{
}

int code_net_rem_memb(struct code_net *h, code_elem_t *to);

int code_link(struct code_net *h, code_elem_t *from, code_elem_t *to);

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

    return que_add(to->in_cmd_queh, cmd);
}

int code_tx_data(code_elem_t *to, void *buf, int len)
{
    return que_add(to->in_data_queh, buf);
}

int code_data_send_hook()
{
    for(;;){
        // check link req's
        // wait on cond for link req to occur
    }
}

/*
 * buf_attr_t = (COPY)
int code_tx_to_grp(int grp_id, buf_attr_t attr, void *buf, int len, void max)
{
}
*/

int code_out_avail(code_elem_t *e, buf_attr_t attr, void *buf, int len, void
        (*cleanup_cb)(void *buf, void *pdata))
{
    struct code_data_req *req;

    req = malloc(sizeof(*req));
    if(!req){
        goto err;
    }

    que_add(e->out_data_queh, req);

err:
  //  int i, c;
  //  code_elem_t *to;

  //  // count first
  // // for(c=0,i=0; i < MAX_LINKS; i++){
  // //     if((e->links[i])){
  // //         c++;
  // //     }
  // // }
  //  struct code_link *curr; 
  //  void *track = NULL;
  //  void *start = ll_get_start(e->linksh); 
  //  printf("zz %p\n", start);

  //  // add to datastructure
  //  ll_foreach(e->linksh, curr, track){
  //      code_tx_data(curr->link, buf, len);
  //  }

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

static void *ctrl_thread(void *arg)
{
    que_get(e->out_data_queh, req);
}

static void *code_thread(void *arg)
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
            buf = que_get(h->in_data_queh, &buf_check_timespec);
            if(buf){
                /* call user function */
                func(h, buf, 1, h->pdata);
            }else{
                if(errno == ETIMEDOUT){
                    /printf("!! timedout xx\n");
                }
            }
            //sleep(1);
        }

        /* incoming commands */
        cmd_buf = que_get(h->in_cmd_queh, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\n ");
            free(cmd_buf);
            break;
            // process and free
        }else{
            if(errno == ETIMEDOUT){
                printf("!! timedout xx\n");
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
        thread_create(&tid, NULL, code_thread, h);
    }else if(h->type == CODE_TYPE_BIN){
        // fork here, with pipe to communicate back
        thread_t tid;
        thread_create(&tid, NULL, code_run_bin, h);
    }
    //}else if(//network ){
    //  make connection
    //}else if(h->type == FORK_PROCESS){
    //}else if(h->type == REMOTE_PROCESS){
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


