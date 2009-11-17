
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
#include "_conn.h"
#include "router.h"


/*  */
struct nn_chan {
    int id;
    struct ll *memb; /* all memb in this grp, for router output */
};

/* each router has one tx and one rx buffer. and a ll
of gpr's with there memb(memb) */
struct nn_router{
    struct ll *grp;
    struct ll *memb; /* all memb's connected to the router, router input */
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    struct que *tx_pkts;   /* pkt's going out, not used */

    mutex_t mutex;
    cond_t cond;
};

/* each router have a ll of grp's that it routes to,
 * loop through them with g in turn set */
#define ROUTER_GRP_ITER_PRE(rt) \
    { \
    assert(rt); \
    int done = 0; \
    struct router_chan_iter *iter = NULL; \
    struct nn_chan *g; \
    iter = router_chan_iter_init(rt); \
    while(!done && !router_chan_iter_next(iter, &g)){ \


#define ROUTER_GRP_ITER_POST(rt) \
    } \
    router_chan_iter_free(iter); \
    }

/* each grp have a ll of memb's it is connected to */
#define GRP_CONN_ITER_PRE(g) \
    { \
    assert(g); \
    int done = 0; \
    struct grp_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    iter = grp_conn_iter_init(g); \
    while(!done && !grp_conn_iter_next(iter, &cn)){ \

#define GRP_CONN_ITER_POST(g) \
    } \
    grp_conn_iter_free(iter); \
    }


/* this type don't exist, just for type checking */
struct router_chan_iter;

static void *router_thread(void *arg);
static int router_conn_free_all(struct nn_router *rt);

static int router_lock(struct nn_router *rt);
static int router_unlock(struct nn_router *rt);
static int router_cond_wait(struct nn_router *rt);
static int router_cond_broadcast(struct nn_router *rt);

static int router_rx_pkts(struct nn_router *rt);
static int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt);

static int router_tx_pkts(struct nn_router *rt);

static struct router_chan_iter *router_chan_iter_init(struct nn_router *rt);
static int router_chan_iter_free(struct router_chan_iter *iter);
static int router_chan_iter_next(struct router_chan_iter *iter, struct nn_chan
        **g);

static struct grp_conn_iter *grp_conn_iter_init(struct nn_chan *g);
static int grp_conn_iter_free(struct grp_conn_iter *iter);
static int grp_conn_iter_next(struct grp_conn_iter *iter, struct nn_conn **cn);

static int router_isvalid(struct nn_router *rt);

/* main loop, check status rx packets and route */
static void *router_thread(void *arg)
{
    //struct timespec ts = {0, 0};
    struct nn_router *rt = arg;

    L(LNOTICE, "Router thread starting");

    thread_detach(thread_self());

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
                router_unlock(rt);
                router_conn_free_all(rt);
                router_set_state(rt, NN_STATE_FINISHED);
                L(LNOTICE, "Route shutdown complete: %p", rt);
                thread_exit(NULL);
                break;
            case NN_STATE_FINISHED:
                L(LCRIT, "Illegal state");
                break;
        }

        router_unlock(rt);

        /* rx packets from memb */
        router_rx_pkts(rt);

        /* tx packet to memb */
        router_tx_pkts(rt);

        sched_yield();

    }
    return NULL;
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

    PCHK(LWARN, rt->grp, ll_init());
    if(!rt->grp){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->memb, ll_init());
    if(!rt->memb){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }


    PCHK(LWARN, rt->rx_pkts, que_init(100));
    if(!(rt->rx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->tx_pkts, que_init(100));
    if(!(rt->tx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }

    // FIXME: err checking
    mutex_init(&rt->mutex, NULL);
    cond_init(&rt->cond, NULL);

    router_isvalid(rt);

    rt->state = NN_STATE_PAUSED;

    /* start router_thread that will do the routing */
    thread_create(&tid, NULL, router_thread, rt);

err:
    return rt;
}

int router_free(struct nn_router *rt)
{
    int r = 0;
    struct nn_pkt *pkt;
    struct timespec ts = {0, 0};
    int fail;

    router_isvalid(rt);

    mutex_lock(&rt->mutex);

    if(rt->grp){

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

        ICHK(LWARN, r, ll_free(rt->memb));
        ICHK(LWARN, r, ll_free(rt->grp));
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

int router_conn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);
    router_lock(rt);

    fail = 0;

    ICHK(LWARN, r, ll_add_front(rt->memb, (void **)&cn));
    if(r) fail++;

    router_unlock(rt);

    ICHK(LWARN, r, _conn_set_router(cn, rt));

    return r;
}

int router_unconn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    router_lock(rt);

    /* free all the grp memb */
    ROUTER_GRP_ITER_PRE(rt);

    fail = 0;
    ICHK(LWARN, r, ll_rem(g->memb, cn));
    if(r) fail++;

    ROUTER_GRP_ITER_POST(rt);

    ICHK(LWARN, r, ll_rem(rt->memb, cn));
    if(r) fail++;

    r = _conn_free_router(cn);

    router_unlock(rt);

    return r;
}

int router_add_chan(struct nn_router *rt, int id)
{
    int r = 1;
    struct nn_chan *g;

    mutex_lock(&rt->mutex);

    PCHK(LWARN, g, calloc(1, sizeof(*g)));
    if(!g){
        goto err;
    }
    g->id = id;

    PCHK(LWARN, g->memb, ll_init());
    if(!g->memb){
        router_rem_chan(rt, id);
        g = NULL;
        goto err;
    }

    /* add newly created grp */
    ICHK(LWARN, r, ll_add_front(rt->grp, (void **)&g));
    if(r){
        PCHK(LWARN, r, router_rem_chan(rt, id));
        goto err;
    }

    r = 0;

err:
    mutex_unlock(&rt->mutex);

    return r;
}

/* return -1 no such group */
int router_rem_chan(struct nn_router *rt, int id)
{
    int r;
    int fail = -1;

    router_lock(rt);
    ROUTER_GRP_ITER_PRE(rt);

    if(g->id == id){

        done = 1;
        fail = 0;

        if(g->memb){
            ICHK(LWARN, r, ll_free(g->memb));
        }

        if(!fail){
            ICHK(LWARN, r, ll_rem(rt->grp, g));
            if(r) fail++;
        }

        free(g);
    }

    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);

    return fail;
}

struct nn_chan *router_get_chan(struct nn_router *rt, int id)
{
    struct nn_chan *_g = NULL;

    router_lock(rt); \
    ROUTER_GRP_ITER_PRE(rt);

    if(g->id == id){
        _g = g;
        done = 1;
    }

    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);

    return _g;
}

int router_add_to_chan(struct nn_router *rt, int grp_id, struct nn_conn *cn)
{
    int r = 1;

    router_lock(rt);
    ROUTER_GRP_ITER_PRE(rt);

    if(g->id == grp_id){
        ICHK(LWARN, r, ll_add_front(g->memb, (void **)&cn));
        done = 1;
    }

    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);

    return r;
}

int router_rem_from_chan(struct nn_router *rt, int grp_id, struct nn_conn *cn)
{
    int r = 1;

    router_lock(rt);
    ROUTER_GRP_ITER_PRE(rt);

    if(g->id == grp_id){
        ICHK(LWARN, r, ll_rem(g->memb, cn));
        done = 1;
    }

    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);

    return r;
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



/* helper functions */

/* free router side of memb */
static int router_conn_free_all(struct nn_router *rt)
{
    int fail = 0;
    int r;
    //struct router_chan_iter *iter;
    //struct nn_conn *cn;

    router_lock(rt);
    ROUTER_GRP_ITER_PRE(rt);

    GRP_CONN_ITER_PRE(g);

    //r = router_unconn(rt, g->id, cn);

    ICHK(LWARN, r, ll_rem(g->memb, cn));
    if(r) fail++;

    r = _conn_free_router(cn);

    GRP_CONN_ITER_POST(g);

    router_unlock(rt);
    router_rem_chan(rt, g->id);
    router_lock(rt);

    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);


/*
    iter = router_chan_iter_init(rt);
    while(!router_chan_iter_next(iter, &cn)){
        r = router_unconn(rt, cn);
    }
    router_chan_iter_free(iter);
*/
    return fail;
}

static int router_tx_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;
    int dest_chan_id;
    int dest_no;
    int c;
    int send_to;


    /* pick pkt's up from router and move to memb */
    while(!router_get_rx_pkt(rt, &pkt)){
        assert(pkt);

        dest_chan_id = pkt_get_dest_chan_id(pkt);
        dest_no = pkt_get_dest_no(pkt);
        c = 0;
        send_to = 0;

        router_lock(rt);
        ROUTER_GRP_ITER_PRE(rt);

        /* route to the dest group */
        if(g->id == dest_chan_id){

            GRP_CONN_ITER_PRE(g);

            /* don't send to sender */
            if(_conn_get_node(cn) != pkt_get_src(pkt)){
                pkt_inc_refcnt(pkt, 1);
                if(!_conn_router_tx_pkt(cn, pkt)){
                    send_to++;
                    if(dest_no && ++c >= dest_no){
                        done = 1;
                    }
                }else{
                    pkt_inc_refcnt(pkt, -1);
                }
            }

            GRP_CONN_ITER_POST(g);

            /* found and routed to dest group so done */
            done = 1;
        }

        ROUTER_GRP_ITER_POST(rt);
        router_unlock(rt);

        if(!send_to){
            /* not send, add back, IMPROVE: */
            router_lock(rt);
            ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
            router_cond_broadcast(rt);
            router_unlock(rt);
        }else{
            /* free for the orignal packet */
            pkt_free(pkt);
        }

    }


    return 0;
}

static int router_rx_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;

    router_lock(rt);
    ROUTER_GRP_ITER_PRE(rt);
    GRP_CONN_ITER_PRE(g);

    /* pick pkt's up from memb and move to rt */
    if(!_conn_router_rx_pkt(cn, &pkt)){
        assert(pkt);
        ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
        router_cond_broadcast(rt);
    }

    GRP_CONN_ITER_POST(g);
    ROUTER_GRP_ITER_POST(rt);
    router_unlock(rt);

    return 0;
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

static struct router_chan_iter *router_chan_iter_init(struct nn_router *rt)
{
    return (struct router_chan_iter *)ll_iter_init(rt->grp);
}

static int router_chan_iter_free(struct router_chan_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_chan_iter_next(struct router_chan_iter *iter, struct nn_chan **g)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)g);
}

static struct grp_conn_iter *grp_conn_iter_init(struct nn_chan *g)
{
    return (struct grp_conn_iter *)ll_iter_init(g->memb);
}

static int grp_conn_iter_free(struct grp_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int grp_conn_iter_next(struct grp_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
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

static int router_isvalid(struct nn_router *rt)
{
    return 0;
}

#if 0


int router_print(struct nn_router *rt)
{

    router_chan_iter_PRE

    printf("router->memb\t");
    printf("rt=%p, cn:%p, n=%p\n", rt, cn, _conn_get_node(cn));

    router_chan_iter_POST

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

#endif

