
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

    //int conn_pkts_no; /* number of conn bufs ready to be picked up */

    mutex_t mutex;
    cond_t cond;
};


/* each router have a ll of chan's that it routes to,
 * loop through them with chan in turn set */
#define ROUTER_CHAN_ITER_PRE(rt) \
    { \
    assert(rt); \
    int done = 0; \
    struct router_chan_iter *iter = NULL; \
    struct nn_chan *chan; \
    iter = router_chan_iter_init(rt); \
    while(!done && !router_chan_iter_next(iter, &chan)){ \


#define ROUTER_CHAN_ITER_POST(rt) \
    } \
    router_chan_iter_free(iter); \
    }

/* each chan have a ll of conn's it is connected to */
#define CHAN_CONN_ITER_PRE(chan) \
    { \
    assert(chan); \
    int done = 0; \
    struct chan_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    iter = chan_conn_iter_init(chan); \
    while(!done && !chan_conn_iter_next(iter, &cn)){ \

#define CHAN_CONN_ITER_POST(chan) \
    } \
    chan_conn_iter_free(iter); \
    }

#define ROUTER_CONN_ITER_PRE(rt) \
    { \
    assert(rt); \
    int done = 0; \
    struct router_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    iter = router_conn_iter_init(rt); \
    while(!done && !router_conn_iter_next(iter, &cn)){ \


#define ROUTER_CONN_ITER_POST(rt) \
    } \
    router_conn_iter_free(iter); \
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
        **chan);

static struct router_conn_iter *router_conn_iter_init(struct nn_router *rt);
static int router_conn_iter_free(struct router_conn_iter *iter);
static int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn
        **conn);

static struct chan_conn_iter *chan_conn_iter_init(struct nn_chan *chan);
static int chan_conn_iter_free(struct chan_conn_iter *iter);
static int chan_conn_iter_next(struct chan_conn_iter *iter, struct nn_conn **cn);

static int router_isvalid(struct nn_router *rt);



/* main loop, check status rx packets and route */
static void *router_thread(void *arg)
{
    //struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    int rx_pkts_no;

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
                return NULL;
                break;
            case NN_STATE_DONE:
                L(LCRIT, "Illegal state");
                break;
        }

        router_rx_pkts(rt);

       // /* route packets to chan->conn's */
        if(rt->rx_pkts_no){
            router_route_pkts(rt);
        }

        rx_pkts_no = rt->rx_pkts_no;

        router_unlock(rt);

        if(rx_pkts_no){
            // yield if there is more rx packets to process
            sched_yield();
        }else{
            usleep(1000);
        }
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

    PCHK(LWARN, rt->rx_pkts, que_init(9999));
    if(!(rt->rx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }
    rt->rx_pkts_no = 0;

    PCHK(LWARN, rt->tx_pkts, que_init(9999));
    if(!(rt->tx_pkts)){
        PCHK(LWARN, r, router_free(rt));
        rt = NULL;
        goto err;
    }
    rt->tx_pkts_no = 0;

    rt->rx_pkts_total = 0;
    rt->tx_pkts_total = 0;
    //rt->conn_pkts_no = 0;

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

int router_unconn(struct nn_router *rt, struct nn_conn *_cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    router_lock(rt);

    /* free all the chan conn */
    ROUTER_CHAN_ITER_PRE(rt);

    CHAN_CONN_ITER_PRE(chan);

    if(cn == _cn){
        ICHK(LWARN, r, ll_rem(chan->conn, cn));
        if(r) fail++;
        done = 1;
    }

    CHAN_CONN_ITER_POST(chan);

    ROUTER_CHAN_ITER_POST(rt);

    ICHK(LWARN, r, ll_rem(rt->conn, _cn));
    if(r) fail++;

    ICHK(LWARN, r, _conn_free_router(_cn));
    if(r) fail++;

    router_unlock(rt);

    return r;
}

int router_add_chan(struct nn_router *rt, int id)
{
    int r = 1;
    struct nn_chan *chan;

    mutex_lock(&rt->mutex);

    PCHK(LWARN, chan, calloc(1, sizeof(*chan)));
    if(!chan){
        goto err;
    }
    chan->id = id;

    PCHK(LWARN, chan->conn, ll_init());
    if(!chan->conn){
        router_rem_chan(rt, id);
        chan = NULL;
        goto err;
    }

    /* add newly created chan */
    ICHK(LWARN, r, ll_add_front(rt->chan, (void **)&chan));
    if(r){
        PCHK(LWARN, r, router_rem_chan(rt, id));
        goto err;
    }

    r = 0;

err:
    mutex_unlock(&rt->mutex);

    return r;
}

#if 0
int router_conn_buf_avail(struct nn_router *rt, int no)
{
    router_lock(rt);

    rt->conn_pkts_no += no;

    router_cond_broadcast(rt);

    router_unlock(rt);

    return rt->conn_pkts_no;
}
#endif

/* return -1 no such group */
int router_rem_chan(struct nn_router *rt, int id)
{
    int r;
    int fail = -1;

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(chan->id == id){

        done = 1;
        fail = 0;

        if(chan->conn){
            ICHK(LWARN, r, ll_free(chan->conn));
        }

        if(!fail){
            ICHK(LWARN, r, ll_rem(rt->chan, chan));
            if(r) fail++;
        }

        free(chan);
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

    if(chan->id == id){
        _g = chan;
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

    if(chan->id == chan_id){
        ICHK(LWARN, r, ll_add_front(chan->conn, (void **)&cn));
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

    if(chan->id == chan_id){
        ICHK(LWARN, r, ll_rem(chan->conn, cn));
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

    CHAN_CONN_ITER_PRE(chan);

    //r = router_unconn(rt, chan->id, cn);

    ICHK(LWARN, r, ll_rem(chan->conn, cn));
    if(r) fail++;


    CHAN_CONN_ITER_POST(chan);

    router_unlock(rt);
    router_rem_chan(rt, chan->id);
    router_lock(rt);

    ROUTER_CHAN_ITER_POST(rt);


    ROUTER_CONN_ITER_PRE(rt);

    printf("!!!!! free rt: %p\n", cn);
    r = _conn_free_router(cn);
    //r = router_unconn(rt, cn);

    ICHK(LWARN, r, ll_rem(rt->conn, cn));
    if(r) fail++;

    ROUTER_CONN_ITER_POST(rt);

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

static int router_rx_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;

    ROUTER_CHAN_ITER_PRE(rt);
    CHAN_CONN_ITER_PRE(chan);

    /* loop through conn's and move pkt's to rt */
    while(!_conn_router_rx_pkt(cn, &pkt)){
        assert(pkt);

        //rt->conn_pkts_no--;

        ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
        rt->rx_pkts_no++;
        rt->rx_pkts_total++;
        router_cond_broadcast(rt);

        pkt_set_state(pkt, PKT_STATE_RT_RX);
    }

    CHAN_CONN_ITER_POST(chan);
    ROUTER_CHAN_ITER_POST(rt);

    return 0;
}


/* rt have to be locked */
static int router_route_pkts(struct nn_router *rt)
{
    struct nn_pkt *pkt;
    int dest_chan_id;
    int dest_no;
    int c;
    int send_to;
    int pick_up = 10;

    /* pick pkt's up from router and move to conn */
    while(--pick_up && !router_get_rx_pkt(rt, &pkt)){
        assert(pkt);

        //if(pkt_cancelled(pkt)){
        //    continue;
        //}

        dest_chan_id = pkt_get_dest_chan_id(pkt);
        dest_no = pkt_get_dest_no(pkt);
        c = 0;
        send_to = -1;

        ROUTER_CHAN_ITER_PRE(rt);

        /* route to the dest group */
        if(chan->id == dest_chan_id){

            send_to = 0;
            CHAN_CONN_ITER_PRE(chan);

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

            CHAN_CONN_ITER_POST(chan);

            rt->tx_pkts_total++;

            /* found and routed to dest group so done */
            done = 1;
        }

        ROUTER_CHAN_ITER_POST(rt);

        pkt_set_state(pkt, PKT_STATE_RT_ROUTED);
        pkt_free(pkt);



        //if(!send_to){
        //    /* not send, add back, IMPROVE: */
        //   // ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
        //   // rt->rx_pkts_no++;
        //   // rt->rx_pkts_total++;
        //   // router_cond_broadcast(rt);
        //}else{
        //    /* free for the orignal packet */
        //    rt->tx_pkts_total++;
        //    pkt_free(pkt);
        //}
    }

    return 0;
}

static int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    //router_lock(rt);

    *pkt = que_get(rt->rx_pkts, &ts);

    //router_unlock(rt);

    if(*pkt){ //&& !pkt_cancelled(*pkt)){
        rt->rx_pkts_no--;
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

static int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **conn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)conn);
}


static struct router_chan_iter *router_chan_iter_init(struct nn_router *rt)
{
    return (struct router_chan_iter *)ll_iter_init(rt->chan);
}

static int router_chan_iter_free(struct router_chan_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_chan_iter_next(struct router_chan_iter *iter, struct nn_chan **chan)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)chan);
}

static struct chan_conn_iter *chan_conn_iter_init(struct nn_chan *chan)
{
    return (struct chan_conn_iter *)ll_iter_init(chan->conn);
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

