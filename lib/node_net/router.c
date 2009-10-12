
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
#include "router.h"


struct nn_router {
    struct ll *memb;

    struct que *in_cmd;
    struct que *in_data;
    struct que *out_cmd;
    struct que *out_data;
    struct que *in_notify;

    /* internal commands */
    struct que *in_int_cmd;
    struct que *out_int_cmd;

    io_cmd_req_cb_t io_in_cmd_cb;
    io_data_req_cb_t io_in_data_cb;
};

/* for nn_router->memb */
struct nn_router_memb {
    struct nn_node *memb;
};

int router_isvalid(struct nn_router *rt)
{
    assert(rt->memb);
}

struct nn_router *router_init()
{
    int err;
    struct nn_router *rt;

    PCHK(LWARN, rt, calloc(1, sizeof(*rt)));
    if(!rt){
        goto err;
    }

    PCHK(LWARN, rt->memb, ll_init());
    if(!rt->memb){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->in_cmd, que_init(32));
    if(!rt->in_cmd){
        router_free(rt);
        rt = NULL;
        goto err;
    }


    PCHK(LWARN, rt->in_data, que_init(32));
    if(!rt->in_data){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    router_isvalid(rt);
err:
    return rt;
}

int router_free(struct nn_router *rt)
{
    int r = 0;
    router_isvalid(rt);

    if(rt->memb){
        ICHK(LWARN, r, ll_free(rt->memb));
    }

    if(rt->in_cmd){
        ICHK(LWARN, r, que_free(rt->in_cmd));
    }

    if(rt->in_data){
        ICHK(LWARN, r, que_free(rt->in_data));
    }


    free(rt);
    return r;
}


int router_add_memb(struct nn_router *rt, struct nn_node *n)
{
    int r = 1;
    struct nn_router_memb *nm;
    router_isvalid(rt);

    PCHK(LWARN, nm, malloc(sizeof(*nm)));
    if(!nm){
        goto err;
    }


    nm->memb = n;
    ICHK(LWARN, r, ll_add_front(rt->memb, (void **)&nm));

err:
    return r;
}

int router_rem_memb(struct nn_router *rt, struct nn_node *n)
{
    int r;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->memb, n));

    return r;
}

int router_ismemb(struct nn_router *rt, struct nn_node *n)
{
    int r = 1;
    struct nn_router_memb *nm;
    void *iter;

    assert(rt);

    iter = NULL;
    while((nm=ll_next(rt->memb, &iter))){
        if(nm->memb == n){
            r = 0;
            break;
        }
    }

    return r;
}



int router_print(struct nn_router *rt)
{
    struct nn_router_memb *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\rt-- router->node --: %p\rt\rt", rt);
    c = 0;

    iter = NULL;
    //ll_each(rt->memb, nm, iter){
    while((nm = ll_next(rt->memb, &iter))){
        printf("zee\rt");
        printf("p:%p\rt", nm->memb);
        c++;
    }

    printf("total: %d\rt\rt", c);

    return 0;
}


int router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb)
{
    router_isvalid(rt);
    rt->io_in_cmd_cb = cb;
    return 0;
}

int router_set_data_cb(struct nn_router *rt, io_data_req_cb_t cb)
{
    router_isvalid(rt);
    //rt->io_in_data_cb = cb;
    return 0;
}

int router_add_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    return que_add(rt->in_cmd, cmd);
}

struct nn_cmd *router_get_cmd(struct nn_router *rt, struct timespec *ts)
{
    return que_get(rt->in_cmd, ts);
}

//int router_add_in_data(struct nn_router *rt, struct nn_data *data)
//{
//    return que_add(rt->in_data, data);
//}


struct nn_node *router_memb_iter(struct nn_router *rt, void **iter)
{
    int r = 0;
    struct nn_router_memb *m;
    struct nn_cmd *clone;

    assert(rt);

    m = ll_next(rt->memb, iter);

    if(m){
        return m->memb;
    }else{
        return NULL;
    }
}
