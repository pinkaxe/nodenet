
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "cmd.h"
#include "link.h"
#include "router.h"


struct nn_router {
    struct ll *link;

   // struct que *in_cmd;
   // struct que *in_data;
   // struct que *out_cmd;
   // struct que *out_data;
   // struct que *in_notify;

   // /* internal commands */
   // struct que *in_int_cmd;
   // struct que *out_int_cmd;

    io_cmd_req_cb_t io_in_cmd_cb;
    io_data_req_cb_t io_in_data_cb;

    mutex_t mutex;
};

/* for nn_router->link */
struct nn_router_link {
    struct nn_link *link;
};

int router_isvalid(struct nn_router *rt)
{
    assert(rt->link);
}

struct nn_router *router_init()
{
    int err;
    struct nn_router *rt;

    PCHK(LWARN, rt, calloc(1, sizeof(*rt)));
    if(!rt){
        goto err;
    }

    PCHK(LWARN, rt->link, ll_init());
    if(!rt->link){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    /*
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
    */

    // FIXME: err checking
    mutex_init(&rt->mutex, NULL);

    router_isvalid(rt);
err:
    return rt;
}

int router_free(struct nn_router *rt)
{
    void *iter;
    struct nn_router_link *rm;
    int r = 0;
    router_isvalid(rt);

    mutex_lock(&rt->mutex);

    if(rt->link){
        iter = NULL;
//
//        if(rm=ll_next(rt->link, &iter)){
//                assert(0 == 1);
//        }


        //while(rm=ll_next(rt->link, &iter)){
        //    ICHK(LWARN, r, ll_rem(rt->link, rm));
        //    //elem_rem_from_grp(rm);
        //    free(rm);
        //}

        //router_unlock_membs(rt);

        ICHK(LWARN, r, ll_free(rt->link));
    }

    /*
    if(rt->in_cmd){
        ICHK(LWARN, r, que_free(rt->in_cmd));
    }

    if(rt->in_data){
        ICHK(LWARN, r, que_free(rt->in_data));
    }
    */

    mutex_unlock(&rt->mutex);
    mutex_destroy(&rt->mutex);

    free(rt);
    return r;
}


int router_lock(struct nn_router *rt)
{
    mutex_lock(&rt->mutex);
    return 0;
}

int router_unlock(struct nn_router *rt)
{
    mutex_unlock(&rt->mutex);
    return 0;
}

int router_conn(struct nn_router *rt, struct nn_link *l)
{
    int r = 1;
    struct nn_router_link *nm;
    router_isvalid(rt);

    PCHK(LWARN, nm, malloc(sizeof(*nm)));
    if(!nm){
        goto err;
    }

    nm->link = l;
    ICHK(LWARN, r, ll_add_front(rt->link, (void **)&nm));

err:
    return r;
}

int router_dconn(struct nn_router *rt, struct nn_link *l)
{
    int r;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->link, l));

    return r;
}

int router_isconn(struct nn_router *rt, struct nn_node *n)
{
    int r = 1;
    struct nn_router_link *nm;
    void *iter;

    assert(rt);

    iter = NULL;
    while((nm=ll_next(rt->link, &iter))){
        // FIXME:
        //if(nm->link->n == n){
        //    r = 0;
        //    break;
        //}
    }

    return r;
}



int router_print(struct nn_router *rt)
{
    struct nn_router_link *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\rt-- router->node --: %p\rt\rt", rt);
    c = 0;

    iter = NULL;
    //ll_each(rt->link, nm, iter){
    while((nm = ll_next(rt->link, &iter))){
        printf("zee\rt");
        printf("p:%p\rt", nm->link);
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


struct nn_link *router_conn_iter(struct nn_router *rt, void **iter)
{
    int r = 0;
    struct nn_router_link *m;
    struct nn_cmd *clone;

    assert(rt);

    m = ll_next(rt->link, iter);

    if(m && m->link){
        return m->link;
    }else{
        return NULL;
    }
}

#if 0
int router_add_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    return que_add(rt->in_cmd, cmd);
}

struct nn_cmd *router_get_cmd(struct nn_router *rt, struct timespec *ts)
{
    return que_get(rt->in_cmd, ts);
}
#endif

//int router_add_in_data(struct nn_router *rt, struct nn_data *data)
//{
//    return que_add(rt->in_data, data);
//}

