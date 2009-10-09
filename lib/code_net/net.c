
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "io.h"
#include "elem.h"
#include "net.h"

struct cn_net_memb {
    struct cn_elem *memb;
};

struct cn_net {
    struct ll *memb;
    struct que *cmd_req;
    io_cmd_req_cb_t io_cmd_req_cb;
    struct que *data_req;
    io_data_req_cb_t io_data_req_cb;
};


int net_isvalid(struct cn_net *n)
{
    assert(n->memb);
}

struct cn_net *net_init()
{
    int err;
    struct cn_net *n;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }

    PCHK(LWARN, n->memb, ll_init());
    if(!n->memb){
        net_free(n);
        n = NULL;
        goto err;
    }

    PCHK(LWARN, n->cmd_req, que_init(32));
    if(!n->cmd_req){
        net_free(n);
        n = NULL;
        goto err;
    }


    PCHK(LWARN, n->data_req, que_init(32));
    if(!n->data_req){
        net_free(n);
        n = NULL;
        goto err;
    }

    net_isvalid(n);
err:
    return n;
}

int net_free(struct cn_net *n)
{
    int r = 0;
    net_isvalid(n);

    if(n->memb){
        ICHK(LWARN, r, ll_free(n->memb));
    }

    if(n->cmd_req){
        ICHK(LWARN, r, que_free(n->cmd_req));
    }

    if(n->data_req){
        ICHK(LWARN, r, que_free(n->data_req));
    }


    free(n);
    return r;
}

int net_add_memb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    net_isvalid(n);

    PCHK(LWARN, nm, malloc(sizeof(*nm)));
    if(!nm){
        goto err;
    }


    nm->memb = e;
    ICHK(LWARN, r, ll_add_front(n->memb, (void **)&nm));

err:
    return r;
}

int net_rem_memb(struct cn_net *n, struct cn_elem *e)
{
    int r;

    net_isvalid(n);

    ICHK(LWARN, r, ll_rem(n->memb, e));

    return r;
}

int net_ismemb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    void *iter;

    assert(n);

    iter = NULL;
    while((nm=ll_next(n->memb, &iter))){
        if(nm->memb == e){
            r = 0;
            break;
        }
    }

    return r;
}


int net_sendto_all(struct cn_net *n, struct cn_io_cmd *cmd)
{
    int r = 0;
    struct cn_net_memb *nm;
    void *iter;
    struct cn_io_cmd *clone;

    assert(n);

    iter = NULL;
    while((nm=ll_next(n->memb, &iter))){
        clone = io_cmd_clone(cmd);
        while((r=send_cmd_to_elem(nm->memb, clone))){
            usleep(100);
        }
        if(r){
            goto err;
        }
    }

err:
    return r;
}


int net_print(struct cn_net *n)
{
    struct cn_net_memb *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\n-- net->elem --: %p\n\n", n);
    c = 0;

    iter = NULL;
    //ll_each(n->memb, nm, iter){
    while((nm = ll_next(n->memb, &iter))){
        printf("zee\n");
        printf("p:%p\n", nm->memb);
        c++;
    }

    printf("total: %d\n\n", c);

    return 0;
}

int net_set_cmd_cb(struct cn_net *n, io_cmd_req_cb_t cb)
{
    net_isvalid(n);
    n->io_cmd_req_cb = cb;
    return 0;
}

int net_add_cmd_req(struct cn_net *n, struct cn_io_cmd *cmd)
{
    return que_add(n->cmd_req, cmd);
}

int net_set_data_cb(struct cn_net *n, io_data_req_cb_t cb)
{
    net_isvalid(n);
    n->io_data_req_cb = cb;
    return 0;
}

int net_add_data_req(struct cn_net *n, struct cn_io_data *data)
{
    return que_add(n->data_req, data);
}

/* thread implementation */
#include "sys/thread.h"

void *cmd_req_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_net *n = arg;
    struct cn_io_cmd *cmd;

    for(;;){
        cmd = que_get(n->cmd_req, NULL);
        if(cmd){
            n->io_cmd_req_cb(n, cmd);
        }

    }
}

void *data_req_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_net *n = arg;
    struct cn_io_data *data;

    for(;;){
        data = que_get(n->data_req, NULL);
        if(data){
            n->io_data_req_cb(n, data);
        }

    }
}

int net_run(struct cn_net *n)
{
    thread_t tid;
    thread_create(&tid, NULL, cmd_req_thread, n);
    thread_detach(tid);

    thread_create(&tid, NULL, data_req_thread, n);
    thread_detach(tid);
}
