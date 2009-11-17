
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
    struct ll *conn; /* all conn in this chan, for router output */
};

/* each router has one tx and one rx buffer. and a ll
of gpr's with there conn(conn) */
struct nn_router{
    struct ll *chan;
    struct ll *conn; /* all conn's connected to the router, router input */
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    int rx_pkts_no;        /* how many packets in que */

    struct que *tx_pkts;   /* pkt's going out, not used */
    int tx_pkts_no;        /* how many packets in que */

    int conn_rx_pkts_no;   /* how many packets on conn to pickup */

    int rx_pkts_total;
    int tx_pkts_total;

    mutex_t mutex;
    cond_t cond;
};


/* each router have a ll of chan's that it routes to,
 * loop through them with g in turn set */
#define ROUTER_CHAN_ITER_PRE(rt) \
    { \
    assert(rt); \
    int done = 0; \
    struct router_chan_iter *iter = NULL; \
    struct nn_chan *g; \
    iter = router_chan_iter_init(rt); \
    while(!done && !router_chan_iter_next(iter, &g)){ \


#define ROUTER_CHAN_ITER_POST(rt) \
    } \
    router_chan_iter_free(iter); \
    }

/* each chan have a ll of conn's it is connected to */
#define CHAN_CONN_ITER_PRE(g) \
    { \
    assert(g); \
    int done = 0; \
    struct chan_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    iter = chan_conn_iter_init(g); \
    while(!done && !chan_conn_iter_next(iter, &cn)){ \

#define CHAN_CONN_ITER_POST(g) \
    } \
    chan_conn_iter_free(iter); \
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

static int router_route_pkts(struct nn_router *rt);

static struct router_chan_iter *router_chan_iter_init(struct nn_router *rt);
static int router_chan_iter_free(struct router_chan_iter *iter);
static int router_chan_iter_next(struct router_chan_iter *iter, struct nn_chan
        **g);

static struct chan_conn_iter *chan_conn_iter_init(struct nn_chan *g);
static int chan_conn_iter_free(struct chan_conn_iter *iter);
static int chan_conn_iter_next(struct chan_conn_iter *iter, struct nn_conn **cn);

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

        //while(router_get_state(rt) == NN_STATE_RUNNING && !rt->rx_pkts_no){
        //    /* rx packets from conn's */
        //    router_rx_pkts(rt);
        //    //usleep(100000);

        //    //router_cond_wait(rt);
        //}

again:
        /* state changed ? */
        switch(router_get_state(rt)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                L(LNOTICE, "Router paused: %p", rt);
                while(router_get_state(rt) == NN_STATE_PAUSED){
                    router_cond_wait(rt);
                }
                L(LNOTICE, "Router unpaused %p", rt);
                goto again;
                break;
            case NN_STATE_SHUTDOWN:
                L(LNOTICE, "Route shutdown start: %p", rt);
                router_unlock(rt);
                router_conn_free_all(rt);
                router_set_state(rt, NN_STATE_DONE);
                L(LNOTICE, "Route shutdown complete: %p", rt);
                thread_exit(NULL);
                break;
            case NN_STATE_DONE:
                L(LCRIT, "Illegal state");
                break;
        }

        router_rx_pkts(rt);

        /* route packets to chan->conn's */
        if(rt->rx_pkts_no){
            router_route_pkts(rt);
        }

        router_unlock(rt);

        usleep(1000);
        //sched_yield();

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

    PCHK(LWARN, rt->chan, ll_init());
    if(!rt->chan){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->conn, ll_init());
    if(!rt->conn){
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
    rt->rx_pkts_no = 0;

    PCHK(LWARN, rt->tx_pkts, que_init(100));
    if(!(rt->tx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }
    rt->tx_pkts_no = 0;

    rt->rx_pkts_total = 0;
    rt->tx_pkts_total = 0;

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

    if(rt->chan){

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

        ICHK(LWARN, r, ll_free(rt->conn));
        ICHK(LWARN, r, ll_free(rt->chan));
    }


    mutex_unlock(&rt->mutex);

    cond_destroy(&rt->cond);
    mutex_destroy(&rt->mutex);

    free(rt);

    return r;
}

int router_clean(struct nn_router *rt)
{
    int r = 0;

    router_lock(rt);

    while(router_get_state(rt) != NN_STATE_DONE){
        router_cond_wait(rt);
    }

    router_unlock(rt);
    router_free(rt);

    return r;
}

int router_conn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);
    router_lock(rt);

    fail = 0;

    ICHK(LWARN, r, ll_add_front(rt->conn, (void **)&cn));
    if(r) fail++;

    router_unlock(rt);

    return r;
}

int router_unconn(struct nn_router *rt, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    router_lock(rt);

    /* free all the chan conn */
    ROUTER_CHAN_ITER_PRE(rt);

    fail = 0;
    ICHK(LWARN, r, ll_rem(g->conn, cn));
    if(r) fail++;

    ROUTER_CHAN_ITER_POST(rt);

    ICHK(LWARN, r, ll_rem(rt->conn, cn));
    if(r) fail++;


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

    PCHK(LWARN, g->conn, ll_init());
    if(!g->conn){
        router_rem_chan(rt, id);
        g = NULL;
        goto err;
    }

    /* add newly created chan */
    ICHK(LWARN, r, ll_add_front(rt->chan, (void **)&g));
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
    ROUTER_CHAN_ITER_PRE(rt);

    if(g->id == id){

        done = 1;
        fail = 0;

        if(g->conn){
            ICHK(LWARN, r, ll_free(g->conn));
        }

        if(!fail){
            ICHK(LWARN, r, ll_rem(rt->chan, g));
            if(r) fail++;
        }

        free(g);
    }

    ROUTER_CHAN_ITER_POST(rt);
    router_unlock(rt);

    return fail;
}

struct nn_chan *router_get_chan(struct nn_router *rt, int id)
{
    struct nn_chan *_g = NULL;

    router_lock(rt); \
    ROUTER_CHAN_ITER_PRE(rt);

    if(g->id == id){
        _g = g;
        done = 1;
    }

    ROUTER_CHAN_ITER_POST(rt);
    router_unlock(rt);

    return _g;
}

int router_add_to_chan(struct nn_router *rt, int chan_id, struct nn_conn *cn)
{
    int r = 1;

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(g->id == chan_id){
        ICHK(LWARN, r, ll_add_front(g->conn, (void **)&cn));
        done = 1;
    }

    ROUTER_CHAN_ITER_POST(rt);
    router_unlock(rt);

    return r;
}

int router_rem_from_chan(struct nn_router *rt, int chan_id, struct nn_conn *cn)
{
    int r = 1;

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(g->id == chan_id){
        ICHK(LWARN, r, ll_rem(g->conn, cn));
        done = 1;
    }

    ROUTER_CHAN_ITER_POST(rt);
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

/* free router side of conn */
static int router_conn_free_all(struct nn_router *rt)
{
    int fail = 0;
    int r;
    //struct router_chan_iter *iter;
    //struct nn_conn *cn;

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    CHAN_CONN_ITER_PRE(g);

    //r = router_unconn(rt, g->id, cn);

    ICHK(LWARN, r, ll_rem(g->conn, cn));
    if(r) fail++;

    r = _conn_free_router(cn);

    CHAN_CONN_ITER_POST(g);

    router_unlock(rt);
    router_rem_chan(rt, g->id);
    router_lock(rt);

    ROUTER_CHAN_ITER_POST(rt);
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

/* rt have to be locked */
static int router_route_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;
    int dest_chan_id;
    int dest_no;
    int c;
    int send_to;
    int pick_up = 10;

    /* pick pkt's up from router and move to conn */
    while(pick_up-- && !router_get_rx_pkt(rt, &pkt)){
        assert(pkt);

        dest_chan_id = pkt_get_dest_chan_id(pkt);
        dest_no = pkt_get_dest_no(pkt);
        c = 0;
        send_to = -1;

        ROUTER_CHAN_ITER_PRE(rt);

        /* route to the dest group */
        if(g->id == dest_chan_id){

            send_to = 0;
            CHAN_CONN_ITER_PRE(g);

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

            CHAN_CONN_ITER_POST(g);

            /* found and routed to dest group so done */
            done = 1;
        }

        ROUTER_CHAN_ITER_POST(rt);

        if(!send_to){
            /* not send, add back, IMPROVE: */
           // ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
           // rt->rx_pkts_no++;
           // rt->rx_pkts_total++;
           // router_cond_broadcast(rt);
        }else{
            /* free for the orignal packet */
            rt->tx_pkts_total++;
            pkt_free(pkt);
        }
    }

    return 0;
}

static int router_rx_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;

    ROUTER_CHAN_ITER_PRE(rt);
    CHAN_CONN_ITER_PRE(g);

    /* loop through conn's and move pkt's to rt */
    while(!_conn_router_rx_pkt(cn, &pkt)){
        assert(pkt);
        ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
        rt->rx_pkts_no++;
        rt->rx_pkts_total++;
        router_cond_broadcast(rt);
    }

    CHAN_CONN_ITER_POST(g);
    ROUTER_CHAN_ITER_POST(rt);

    return 0;
}

static int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    //router_lock(rt);

    *pkt = que_get(rt->rx_pkts, &ts);

    //router_unlock(rt);

    if(*pkt){
        rt->rx_pkts_no--;
        L(LDEBUG, "+ router_get_rx_pkt %p", *pkt);
        r = 0;
    }

    return r;
}

static struct router_chan_iter *router_chan_iter_init(struct nn_router *rt)
{
    return (struct router_chan_iter *)ll_iter_init(rt->chan);
}

static int router_chan_iter_free(struct router_chan_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_chan_iter_next(struct router_chan_iter *iter, struct nn_chan **g)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)g);
}

static struct chan_conn_iter *chan_conn_iter_init(struct nn_chan *g)
{
    return (struct chan_conn_iter *)ll_iter_init(g->conn);
}

static int chan_conn_iter_free(struct chan_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int chan_conn_iter_next(struct chan_conn_iter *iter, struct nn_conn **cn)
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

int router_get_status(struct nn_router *rt, struct router_status *status)
{
    router_lock(rt);

    status->rx_pkts_no = rt->rx_pkts_no;
    status->tx_pkts_no = rt->tx_pkts_no;
    status->rx_pkts_total = rt->rx_pkts_total;
    status->tx_pkts_total = rt->tx_pkts_total;

    router_unlock(rt);

    return 0;
}


#if 0


int router_print(struct nn_router *rt)
{

    router_chan_iter_PRE

    printf("router->conn\t");
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

