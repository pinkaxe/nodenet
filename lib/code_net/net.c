
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
#include "cmd.h"
#include "net.h"

struct cn_net_memb {
    struct cn_elem *memb;
};

struct cn_net {
    struct ll *memb;
    struct que *cmd_req;
    struct que *data_req;
    io_cmd_req_cb_t io_cmd_req_cb;
    //io_data_req_cb_t io_data_req_cb;
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

int net_set_data_cb(struct cn_net *n, io_data_req_cb_t cb)
{
    net_isvalid(n);
    //n->io_data_req_cb = cb;
    return 0;
}

int net_add_cmd(struct cn_net *n, struct cn_cmd *cmd)
{
    return que_add(n->cmd_req, cmd);
}

struct cn_cmd *net_get_cmd(struct cn_net *n, struct timespec *ts)
{
    return que_get(n->cmd_req, ts);
}

//int net_add_data_req(struct cn_net *n, struct cn_data *data)
//{
//    return que_add(n->data_req, data);
//}


struct cn_elem *net_memb_iter(struct cn_net *n, void **iter)
{
    int r = 0;
    struct cn_net_memb *m;
    struct cn_cmd *clone;

    assert(n);

    m = ll_next(n->memb, iter);

    if(m){
        return m->memb;
    }else{
        return NULL;
    }
}
