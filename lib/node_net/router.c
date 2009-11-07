
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


/* easy iterator pre/post
 * rt != NULL when this is called, afterwards cn for the
 matching each matching node is set and can be used */
#define ROUTER_CONN_ITER_PRE \
    { \
    assert(rt); \
    int done = 0; \
    struct router_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    router_lock(rt); \
    iter = router_conn_iter_init(rt); \
    while(!done && !router_conn_iter_next(iter, &cn)){ \
        conn_lock(cn);

#define ROUTER_CONN_ITER_POST \
        conn_unlock(cn); \
    } \
    router_conn_iter_free(iter); \
    router_unlock(rt); \
    }


struct nn_router {
    struct ll *conn; /* all the conn's */
    //struct que *conn_data_avail; /* conn with data available */
    io_pkt_req_cb_t io_in_pkt_cb;
    io_data_req_cb_t io_in_data_cb;

    struct que *rx_pkts;   /* pkt's coming in */
    struct que *tx_pkts;   /* pkt's going out */

    enum nn_state state;

    mutex_t mutex;
    cond_t cond;
};

/* this type don't exist, just for type checking */
struct router_conn_iter;

static void *router_thread(void *arg);
static int router_conn_free_all(struct nn_router *rt);

static int router_lock(struct nn_router *rt);
static int router_unlock(struct nn_router *rt);
static int router_cond_wait(struct nn_router *rt);
static int router_cond_broadcast(struct nn_router *rt);

static int router_rx_pkts(struct nn_router *rt);
static int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt);

static int router_add_tx_pkt(struct nn_router *rt, struct nn_pkt *pkt);
static int router_tx_pkts(struct nn_router *rt);
static int router_get_tx_pkt(struct nn_router *rt, struct nn_pkt **pkt);

static struct router_conn_iter *router_conn_iter_init(struct nn_router *rt);
static int router_conn_iter_free(struct router_conn_iter *iter);
static int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **cn);
static int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn, void *a0), 
        struct nn_pkt *pkt);



int router_isvalid(struct nn_router *rt)
{
    assert(rt->conn);
    return 0;
}


struct nn_router *router_init()
{
    int r;
    struct nn_router *rt;
    thread_t tid;

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

    PCHK(LWARN, rt->rx_pkts, que_init(100));
    if(!(rt->rx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        goto err;
    }

    PCHK(LWARN, rt->tx_pkts, que_init(100));
    if(!(rt->tx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        goto err;
    }

    // FIXME: err checking
    mutex_init(&rt->mutex, NULL);
    cond_init(&rt->cond, NULL);

    rt->state = NN_STATE_PAUSED;

    router_isvalid(rt);

    /* start router_thread */
    thread_create(&tid, NULL, router_thread, rt);
    thread_detach(tid);
err:
    return rt;
}



int router_free(struct nn_router *rt)
{
    int r = 0;
    int fail = 0;
    struct nn_pkt *pkt;
    struct timespec ts = {0, 0};

    router_isvalid(rt);

    mutex_lock(&rt->mutex);

    if(rt->conn){
        ICHK(LWARN, r, ll_free(rt->conn));
    }

    if(rt->rx_pkts){
        while((pkt=que_get(rt->rx_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(rt->rx_pkts));
        if(r) fail++;
    }

    if(rt->tx_pkts){
        while((pkt=que_get(rt->tx_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(rt->tx_pkts));
        if(r) fail++;
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

    state = rt->state;

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
    int r = 1;

    router_isvalid(rt);

    ICHK(LWARN, r, ll_rem(rt->conn, cn));
    if(r) goto err;

    r = conn_free_router(cn);

err:
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

int router_add_rx_pkt(struct nn_router *rt, struct nn_pkt *pkt)
{
    int r;

    assert(pkt);

    router_lock(rt);

    ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));

    router_unlock(rt);

    L(LDEBUG, "+ router_add_rx_pkt %p(%d)", pkt, r);

    return r;
}



/* pick up pkts coming to router, and router to other node's, main loop */
static void *router_thread(void *arg)
{
    //struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_pkt *pkt;

    L(LNOTICE, "Router thread starting");

    for(;;){

        router_lock(rt);
again:
        /* state changed ? */
        switch(router_get_state(rt)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                L(LNOTICE, "Router paused: %p", rt);
                while(router_get_state(rt) == NN_STATE_PAUSED){
                    router_unlock(rt);
                    //router_cond_wait(rt);
                    sleep(1);
                    router_lock(rt);
                }
                L(LNOTICE, "Router unpaused %p", rt);
                goto again;
                break;
            case NN_STATE_SHUTDOWN:
                L(LNOTICE, "Route shutdown start: %p", rt);
                router_conn_free_all(rt);
                router_unlock(rt);
                router_set_state(rt, NN_STATE_FINISHED);
                L(LNOTICE, "Route shutdown complete: %p", rt);
                thread_exit(NULL);
                break;
            case NN_STATE_FINISHED:
                L(LCRIT, "Illegal state");
                break;
        }

        router_unlock(rt);

        /* rx packets from conn */
        router_rx_pkts(rt);

        /* route to appropriate nodes */
        while(!router_get_rx_pkt(rt, &pkt)){
            assert(pkt);
            router_add_tx_pkt(rt, pkt);
        }

        /* tx packet to conn */
        router_tx_pkts(rt);

        sched_yield();

    }
    return NULL;
}


/* free router side of conn */
static int router_conn_free_all(struct nn_router *rt)
{
    int r = 0;
    struct router_conn_iter *iter;
    struct nn_conn *cn;

    iter = router_conn_iter_init(rt);
    while(!router_conn_iter_next(iter, &cn)){
        r = router_unconn(rt, cn);
    }
    router_conn_iter_free(iter);
    return 0;
}


static int router_tx_pkts(struct nn_router *rt)
{
    struct nn_pkt *pkt, *clone;

    /* pick pkt's up from router and move to conn */
    while(!router_get_tx_pkt(rt, &pkt)){
        assert(pkt);

        // FIXME: sending to all conn's

        ROUTER_CONN_ITER_PRE
        clone = pkt_clone(pkt);
        while(conn_router_tx_pkt(cn, clone)){
            conn_unlock(cn);
            usleep(10000);
            conn_lock(cn);
        }
        ROUTER_CONN_ITER_POST

        pkt_free(pkt);
    }

    return 0;
}

static int router_rx_pkts(struct nn_router *rt)
{
    struct nn_pkt *pkt;

    ROUTER_CONN_ITER_PRE

    /* pick pkt's up from router and move to conn */
    if(!conn_router_rx_pkt(cn, &pkt)){
        assert(pkt);

        conn_unlock(cn);
        router_unlock(rt);
        // FIXME: sending to all routers
        //ROUTER_CONN_ITER_PRE
        router_add_rx_pkt(rt, pkt);

        router_lock(rt);
        conn_lock(cn);
        //ROUTER_CONN_ITER_POST
    }

    ROUTER_CONN_ITER_POST

    return 0;
}

static int router_add_tx_pkt(struct nn_router *rt, struct nn_pkt *pkt)
{
    int r;

    assert(pkt);

    router_lock(rt);

    ICHK(LWARN, r, que_add(rt->tx_pkts, pkt));

    router_unlock(rt);

    L(LDEBUG, "+ router_add_tx_pkt %p(%d)", pkt, r);

    return r;
}

static int router_get_tx_pkt(struct nn_router *rt, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    router_lock(rt);

    *pkt = que_get(rt->tx_pkts, &ts);

    router_unlock(rt);

    if(*pkt){
        L(LDEBUG, "+ router_get_tx_pkt %p", *pkt);
        r = 0;
    }


    return r;
}

static int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    router_lock(rt);

    *pkt = que_get(rt->rx_pkts, &ts);

    router_unlock(rt);

    if(*pkt){
        L(LDEBUG, "+ router_get_rx_pkt %p", *pkt);
        r = 0;
    }

    return r;
}

static struct router_conn_iter *router_conn_iter_init(struct nn_router *rt)
{
    return (struct router_conn_iter *)ll_iter_init(rt->conn);
}

static int router_conn_iter_free(struct router_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}

static int router_conn_each(struct nn_router *rt,
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

static int router_lock(struct nn_router *rt)
{
    mutex_lock(&rt->mutex);
    return 0;
}

static int router_unlock(struct nn_router *rt)
{
    mutex_unlock(&rt->mutex);
    return 0;
}

static int router_cond_wait(struct nn_router *rt)
{
    cond_wait(&rt->cond, &rt->mutex);
    return 0;
}

static int router_cond_broadcast(struct nn_router *rt)
{
    cond_broadcast(&rt->cond);
    return 0;
}
