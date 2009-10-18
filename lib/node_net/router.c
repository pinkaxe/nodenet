
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
#include "conn.h"
#include "router.h"


struct nn_router {
    struct ll *conn;
    io_cmd_req_cb_t io_in_cmd_cb;
    io_data_req_cb_t io_in_data_cb;

    enum nn_state state;

    mutex_t mutex;
    cond_t cond;
};

int router_isvalid(struct nn_router *rt)
{
    assert(rt->conn);
}

struct nn_router *router_init()
{
    int err;
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

int router_free(struct nn_router *rt)
{
    void *iter;
    struct nn_conn *cn;
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

int router_cond_wait(struct nn_router *rt)
{
    cond_wait(&rt->cond, &rt->mutex);
}

int router_set_state(struct nn_router *rt, enum nn_state state)
{
    rt->state = state;
    return 0;
}

enum nn_state router_get_state(struct nn_router *rt)
{
    return rt->state;
}

int router_conn(struct nn_router *rt, struct nn_conn *cn)
{
    int r = 1;
    router_isvalid(rt);

    ICHK(LWARN, r, ll_add_front(rt->conn, (void **)&cn));

err:
    return r;
}

int router_unconn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->conn, cn));

    return r;
}

// FIXME: make this conn_exist(n, rt);
//int router_isconn(struct nn_router *rt, struct nn_node *n)
//{
//    int r = 1;
//    struct nn_conn *cn;
//    void *iter;
//
//    assert(rt);
//
//    iter = NULL;
//    while((cn=ll_iter_next(rt->conn, &iter))){
//        if(cn->n == n){
//            r = 0;
//            break;
//        }
//        // FIXME:
//        //if(cn->conn->n == n){
//        //    r = 0;
//        //    break;
//        //}
//    }
//
//    return r;
//}



int router_print(struct nn_router *rt)
{
    struct nn_conn *cn;
    int r = 0;
    int c;
    struct ll_iter *iter;

    printf("\rt-- router->node --: %p\rt\rt", rt);
    c = 0;

    //iter = NULL;
    ////ll_each(rt->conn, cn, iter){
    //while((cn = ll_iter_next(rt->conn, &iter))){
    //    printf("zee\rt");
    //    printf("p:%p\rt", cn);
    //    c++;
    //}

    iter = ll_iter_init(rt->conn);
    while((!ll_iter_next(iter, &cn))){
        printf("p:%p\n", cn);
        c++;
    }
    ll_iter_free(iter);

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
    return ll_iter_next((struct ll_iter *)iter, cn);
}

int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn))
{
    int r;
    assert(rt);
    struct router_conn_iter *iter;
    struct nn_conn *cn;

    router_lock(rt);

    iter = router_conn_iter_init(rt);
    while(!router_conn_iter_next(iter, &cn)){
        conn_lock(cn);
        r = cb(cn);
        conn_unlock(cn);
        if(r){
            break;
        }
    }
    router_conn_iter_free(iter);

    router_unlock(rt);
}
