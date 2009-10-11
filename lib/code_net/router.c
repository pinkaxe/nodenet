
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

struct cn_router_memb {
    struct cn_elem *memb;
};

struct cn_router {
    struct ll *memb;
    struct que *cmd_req;
    struct que *data_req;
    io_cmd_req_cb_t io_cmd_req_cb;
    //io_data_req_cb_t io_data_req_cb;
};

int router_isvalid(struct cn_router *rt)
{
    assert(rt->memb);
}

struct cn_router *router_init()
{
    int err;
    struct cn_router *rt;

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

    PCHK(LWARN, rt->cmd_req, que_init(32));
    if(!rt->cmd_req){
        router_free(rt);
        rt = NULL;
        goto err;
    }


    PCHK(LWARN, rt->data_req, que_init(32));
    if(!rt->data_req){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    router_isvalid(rt);
err:
    return rt;
}

int router_free(struct cn_router *rt)
{
    int r = 0;
    router_isvalid(rt);

    if(rt->memb){
        ICHK(LWARN, r, ll_free(rt->memb));
    }

    if(rt->cmd_req){
        ICHK(LWARN, r, que_free(rt->cmd_req));
    }

    if(rt->data_req){
        ICHK(LWARN, r, que_free(rt->data_req));
    }


    free(rt);
    return r;
}


int router_add_memb(struct cn_router *rt, struct cn_elem *e)
{
    int r = 1;
    struct cn_router_memb *nm;
    router_isvalid(rt);

    PCHK(LWARN, nm, malloc(sizeof(*nm)));
    if(!nm){
        goto err;
    }


    nm->memb = e;
    ICHK(LWARN, r, ll_add_front(rt->memb, (void **)&nm));

err:
    return r;
}

int router_rem_memb(struct cn_router *rt, struct cn_elem *e)
{
    int r;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->memb, e));

    return r;
}

int router_ismemb(struct cn_router *rt, struct cn_elem *e)
{
    int r = 1;
    struct cn_router_memb *nm;
    void *iter;

    assert(rt);

    iter = NULL;
    while((nm=ll_next(rt->memb, &iter))){
        if(nm->memb == e){
            r = 0;
            break;
        }
    }

    return r;
}



int router_print(struct cn_router *rt)
{
    struct cn_router_memb *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\rt-- router->elem --: %p\rt\rt", rt);
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


int router_set_cmd_cb(struct cn_router *rt, io_cmd_req_cb_t cb)
{
    router_isvalid(rt);
    rt->io_cmd_req_cb = cb;
    return 0;
}

int router_set_data_cb(struct cn_router *rt, io_data_req_cb_t cb)
{
    router_isvalid(rt);
    //rt->io_data_req_cb = cb;
    return 0;
}

int router_add_cmd(struct cn_router *rt, struct cn_cmd *cmd)
{
    return que_add(rt->cmd_req, cmd);
}

struct cn_cmd *router_get_cmd(struct cn_router *rt, struct timespec *ts)
{
    return que_get(rt->cmd_req, ts);
}

//int router_add_data_req(struct cn_router *rt, struct cn_data *data)
//{
//    return que_add(rt->data_req, data);
//}


struct cn_elem *router_memb_iter(struct cn_router *rt, void **iter)
{
    int r = 0;
    struct cn_router_memb *m;
    struct cn_cmd *clone;

    assert(rt);

    m = ll_next(rt->memb, iter);

    if(m){
        return m->memb;
    }else{
        return NULL;
    }
}
