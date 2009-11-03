
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "pkt.h"
#include "conn.h"
#include "router.h"


struct nn_router {
    struct ll *conn; /* all the conn's */
    //struct que *conn_data_avail; /* conn with data available */
    io_pkt_req_cb_t io_in_pkt_cb;
    io_data_req_cb_t io_in_data_cb;

    enum nn_state state;

    mutex_t mutex;
    cond_t cond;
};


int router_isvalid(struct nn_router *rt)
{
    assert(rt->conn);
    return 0;
}

struct nn_router *router_init()
{
    struct nn_router *rt;

    PCHK(LWARN, rt, calloc(1, sizeof(*rt)));
    if(!rt){
        goto err;
    }

    PCHK(LWARN, rt->conn, ll_init());
    if(!rt->conn){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    // FIXME: err checking
    mutex_init(&rt->mutex, NULL);
    cond_init(&rt->cond, NULL);

    rt->state = NN_STATE_PAUSED;

    router_isvalid(rt);
    router_io_run(rt);
err:
    return rt;
}

int router_lock(struct nn_router *rt)
{
printf("!!! lock\n");
    mutex_lock(&rt->mutex);
    return 0;
}

int router_unlock(struct nn_router *rt)
{
    mutex_unlock(&rt->mutex);
    return 0;
}

int router_cond_wait(struct nn_router *rt)
{
    cond_wait(&rt->cond, &rt->mutex);
    return 0;
}

int router_cond_broadcast(struct nn_router *rt)
{
    cond_broadcast(&rt->cond);
    return 0;
}


int router_free(struct nn_router *rt)
{
    int r = 0;

    router_isvalid(rt);

    mutex_lock(&rt->mutex);

    if(rt->conn){
        ICHK(LWARN, r, ll_free(rt->conn));
    }

    mutex_unlock(&rt->mutex);

    cond_destroy(&rt->cond);
    mutex_destroy(&rt->mutex);

    free(rt);

    return r;
}


int router_clean(struct nn_router *rt)
{
    router_lock(rt);

    while(router_get_state(rt) != NN_STATE_FINISHED){
        router_cond_wait(rt);
    }

    router_unlock(rt);
    router_free(rt);

    return 0;
}



int router_set_state(struct nn_router *rt, enum nn_state state)
{
    router_lock(rt);

    rt->state = state;

    router_cond_broadcast(rt);
    router_unlock(rt);

    return 0;
}

enum nn_state router_get_state(struct nn_router *rt)
{
    enum nn_state state;
    router_isvalid(rt);

    router_lock(rt);
    state = rt->state;
    router_unlock(rt);

    return state;
}

int router_conn(struct nn_router *rt, struct nn_conn *cn)
{
    int r = 1;

    router_isvalid(rt);

    router_lock(rt);
    conn_lock(cn);

    ICHK(LWARN, r, ll_add_front(rt->conn, (void **)&cn));
    if(r) goto err;

    ICHK(LWARN, r, conn_set_router(cn, rt));
    if(r) goto err;

err:
    conn_unlock(cn);
    router_unlock(rt);

    return r;
}

int router_unconn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->conn, cn));

    return r;
}


int router_print(struct nn_router *rt)
{

    ROUTER_CONN_ITER_PRE

    printf("router->conn\t");
    printf("rt=%p, cn:%p, n=%p\n", rt, cn, conn_get_node(cn));

    ROUTER_CONN_ITER_POST

    return 0;
}


int router_set_pkt_cb(struct nn_router *rt, io_pkt_req_cb_t cb)
{
    router_isvalid(rt);
    rt->io_in_pkt_cb = cb;
    return 0;
}

int router_set_data_cb(struct nn_router *rt, io_data_req_cb_t cb)
{
    router_isvalid(rt);
    //rt->io_in_data_cb = cb;
    return 0;
}


struct router_conn_iter *router_conn_iter_init(struct nn_router *rt)
{
    return (struct router_conn_iter *)ll_iter_init(rt->conn);
}

int router_conn_iter_free(struct router_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}

int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn, void *a0),
        struct nn_pkt *pkt)
{
    int r;
    assert(rt);
    struct router_conn_iter *iter;
    struct nn_conn *cn;

    router_lock(rt);

    iter = router_conn_iter_init(rt);
    while(!router_conn_iter_next(iter, &cn)){
        conn_lock(cn);
        r = cb(cn, pkt);
        conn_unlock(cn);
        if(r){
            break;
        }
    }
    router_conn_iter_free(iter);

    router_unlock(rt);

    return r;

}
