
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "sys/thread.h"

#include "log/log.h"
#include "que/que.h"
#include "ll/ll.h"
#include "dpool/dpool.h"

#include "types.h"
#include "pkt.h"
#include "router.h"
#include "node.h"

struct chan_nodes_iter;
struct router_nodes_iter;

/*  */
struct nn_chan {
    int id;
    struct ll *nodes; /* all nodes in this chan, for router output */
    struct dpool *dpool;
};


/* each router has one tx and one rx buffer. and a ll
of gpr's with there nodes(nodes) */
struct nn_router{
    struct ll *chan;
    struct ll *nodes;
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    int rx_pkts_no;        /* how many packets in que */

    struct que *tx_pkts;   /* pkt's going out, not used */
    int tx_pkts_no;        /* how many packets in que */

    int nodes_rx_pkts_no;   /* how many packets on nodes to pickup */

    int rx_pkts_total;
    int tx_pkts_total;

    //int nodes_pkts_no; /* number of nodes bufs ready to be picked up */

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

/* each chan have a ll of nodes's it is nodesected to */
#define CHAN_NODES_ITER_PRE(chan) \
    { \
    assert(chan); \
    int done = 0; \
    struct chan_nodes_iter *iter = NULL; \
    struct nn_node *n; \
    iter = chan_nodes_iter_init(chan); \
    while(!done && !chan_nodes_iter_next(iter, &n)){ \

#define CHAN_NODES_ITER_POST(chan) \
    } \
    chan_nodes_iter_free(iter); \
    }

#define ROUTER_NODES_ITER_PRE(rt) \
    { \
    assert(rt); \
    int done = 0; \
    struct router_nodes_iter *iter = NULL; \
    struct nn_node *n; \
    iter = router_nodes_iter_init(rt); \
    while(!done && !router_nodes_iter_next(iter, &n)){ \


#define ROUTER_NODES_ITER_POST(rt) \
    } \
    router_nodes_iter_free(iter); \
    }


/* this type don't exist, just for type checking */
struct router_chan_iter;

static void *router_thread(void *arg);

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

static struct router_nodes_iter *router_nodes_iter_init(struct nn_router *rt);
static int router_nodes_iter_free(struct router_nodes_iter *iter);
static int router_nodes_iter_next(struct router_nodes_iter *iter, struct nn_node
        **node);

static struct chan_nodes_iter *chan_nodes_iter_init(struct nn_chan *chan);
static int chan_nodes_iter_free(struct chan_nodes_iter *iter);
static int chan_nodes_iter_next(struct chan_nodes_iter *iter, struct nn_node **n);

static int router_isvalid(struct nn_router *rt);


/* internal free */
static int _router_free(struct nn_router *rt);

/* main loop, check status rx packets and route */
static void *router_thread(void *arg)
{
    //struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    int rx_pkts_no;
    int i;

    L(LNOTICE, "Router thread starting");

    thread_detach(thread_self());

    for(i=0;; i++){

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
                L(LNOTICE, "Router shutdown start: %p", rt);
                router_unlock(rt);
                router_set_state(rt, NN_STATE_DONE);
                L(LNOTICE, "Router shutdown complete: %p", rt);
                return NULL;
                break;
            case NN_STATE_DONE:
                L(LCRIT, "Illegal state");
                break;
        }

        if(rt->rx_pkts_no < 9999){
            router_rx_pkts(rt);
        }

       // /* route packets to chan->nodes's */
        //L(LDEBUG, "rt->rx_pkts_no %d", rt->rx_pkts_no);
        if(rt->rx_pkts_no){
            router_route_pkts(rt);
        }

        rx_pkts_no = rt->rx_pkts_no;

        router_unlock(rt);

        if(rx_pkts_no){
            /* more packets to do */
            //sched_yield();
        }else{
            usleep(2000);
        }

        //if(i % 4){
        //    usleep(10000);
        //}
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
        PCHK(LWARN, r, _router_free(rt));
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->nodes, ll_init());
    if(!rt->nodes){
        PCHK(LWARN, r, _router_free(rt));
        rt = NULL;
        goto err;
    }

    PCHK(LWARN, rt->rx_pkts, que_init(9999));
    if(!(rt->rx_pkts)){
        PCHK(LWARN, r, _router_free(rt));
        rt = NULL;
        goto err;
    }
    rt->rx_pkts_no = 0;

    PCHK(LWARN, rt->tx_pkts, que_init(9999));
    if(!(rt->tx_pkts)){
        PCHK(LWARN, r, _router_free(rt));
        rt = NULL;
        goto err;
    }
    rt->tx_pkts_no = 0;

    rt->rx_pkts_total = 0;
    rt->tx_pkts_total = 0;
    //rt->nodes_pkts_no = 0;

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

static int _router_free(struct nn_router *rt)
{
    int r = 0;
    struct nn_pkt *pkt;
    struct timespec ts = {0, 0};
    int fail;

    router_isvalid(rt);

    router_lock(rt);

    /* free all packets */
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

    /* for each channel .. */
    ROUTER_CHAN_ITER_PRE(rt);

        /* remove all node connections */
        CHAN_NODES_ITER_PRE(chan);
            ICHK(LWARN, r, ll_rem(chan->nodes, n));
            if(r) fail++;
        CHAN_NODES_ITER_POST(chan);

        /* then free the chan->nodes ll itself */
        if(chan->nodes){
            ICHK(LWARN, r, ll_free(chan->nodes));
        }

        /* remove the channel */
        ICHK(LWARN, r, ll_rem(rt->chan, chan));
        if(r) fail++;

        /* free the chan */
        free(chan);

    ROUTER_CHAN_ITER_POST(rt);

    ICHK(LWARN, r, ll_free(rt->chan));


    ROUTER_NODES_ITER_PRE(rt);

        /* rem from the rt nodes connections */
        ICHK(LWARN, r, ll_rem(rt->nodes, n));
        if(r) fail++;

    ROUTER_NODES_ITER_POST(rt);

    ICHK(LWARN, r, ll_free(rt->nodes));


    router_unlock(rt);

    cond_destroy(&rt->cond);
    mutex_destroy(&rt->mutex);

    free(rt);

    return r;
}

int router_free(struct nn_router *rt)
{
    int r = 0;

    router_lock(rt);

    while(router_get_state(rt) != NN_STATE_DONE){
        router_cond_wait(rt);
    }

    router_unlock(rt);
    _router_free(rt);

    return r;
}

int router_add_node(struct nn_router *rt, struct nn_node *n)
{
    int r;
    int fail = -1;

    router_isvalid(rt);
    router_lock(rt);

    fail = 0;

    ICHK(LWARN, r, ll_add_front(rt->nodes, (void **)&n));
    if(r) fail++;

    node_inc_refcnt(n, 1);

    router_unlock(rt);

    return r;
}

int router_rem_node(struct nn_router *rt, struct nn_node *_n)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    router_lock(rt);

    /* free all the chan ll pointers to n */
    ROUTER_CHAN_ITER_PRE(rt);

    CHAN_NODES_ITER_PRE(chan);

    if(n == _n){
        ICHK(LWARN, r, ll_rem(chan->nodes, n));
        if(r) fail++;
        done = 1;
    }


    CHAN_NODES_ITER_POST(chan);

    ROUTER_CHAN_ITER_POST(rt);

    node_inc_refcnt(_n, -1);

    ICHK(LWARN, r, ll_rem(rt->nodes, _n));
    if(r) fail++;

    router_unlock(rt);


    return r;
}


int router_add_chan(struct nn_router *rt, struct nn_chan *chan)
{
    int r = 1;

    mutex_lock(&rt->mutex);

/*
    PCHK(LWARN, chan, calloc(1, sizeof(*chan)));
    if(!chan){
        goto err;
    }
    chan->id = id;

    PCHK(LWARN, chan->nodes, ll_init());
    if(!chan->nodes){
        router_rem_chan(rt, id);
        chan = NULL;
        goto err;
    }
*/

    /* add newly created chan */
    ICHK(LWARN, r, ll_add_front(rt->chan, (void **)&chan));
    if(r){
        //PCHK(LWARN, r, router_rem_chan(rt, id));
        goto err;
    }

    r = 0;

err:
    mutex_unlock(&rt->mutex);

    return r;
}

struct nn_chan *chan_init(int size)
{
    struct nn_chan *chan = NULL;

    PCHK(LWARN, chan, calloc(1, sizeof(*chan)));
    if(!chan){
        goto err;
    }
    //chan->id = id;

    PCHK(LWARN, chan->nodes, ll_init());
    if(!chan->nodes){
        free(chan);
        chan = NULL;
        goto err;
    }

    chan->dpool = dpool_create(sizeof(void *), size, 0);
    if(!chan->dpool){
        free(chan->nodes);
        free(chan);
        chan = NULL;
        goto err;
    }

err:
    return chan;

}

static int dpool_free_cb(void *dpool, void *buf)
{
    L(LDEBUG, "dpool_free_cb called");
    struct dpool_buf *dpool_buf = buf;
    free(dpool_buf->data);
    return dpool_ret_buf(dpool, dpool_buf);
}

int chan_add_data(struct nn_chan *chan, struct nn_node *n, void *data, size_t len)
{
    int r;
    struct dpool_buf *dpool_buf;
    struct nn_pkt *pkt;

    /* send packets */
    if((dpool_buf=dpool_get_buf(chan->dpool))){
        dpool_buf->data = malloc(len);
        dpool_buf->len = len;
        memcpy(dpool_buf->data, data, len);

        /* build packet */
        PCHK(LDEBUG, pkt, pkt_init(n, chan, 0, dpool_buf,
                    sizeof(*dpool_buf), chan->dpool, dpool_free_cb));
        assert(pkt);

        if(!node_tx(n, pkt)){
            //usleep(100000);
            //pkt_set_state(pkt, PKT_STATE_CANCELLED);
            //printf("!!! sent\n");
        }else{
            pkt_free(pkt);
        }
        r = 0;
    }else{
        // no buffer available */
        L(LWARN, "!! no buffer available");
        //usleep(100);
        r = 1;
    }
    return r;
}

/* TODO: this is checking all channels */
void *chan_get_data(struct nn_chan *chan, struct nn_node *n)
{
    struct dpool_buf *dpool_buf;
    struct nn_pkt *pkt;
    void *data;

    // get incoming on channel and write to socket
    if(node_rx(n, &pkt) == 0){
        /* incoming packet */
        dpool_buf = pkt_get_data(pkt);

        //L(LNOTICE, "Got buf->i=%d", buf->i);
        L(LNOTICE, "Got buf=%s", dpool_buf->data);
        data = malloc(dpool_buf->len);
        memcpy(data, dpool_buf->data, dpool_buf->len);
        //EAGAIN
        pkt_free(pkt);
    }else{
        data = NULL;
        //L(LDEBUG, "nothing");
    }
    return data;
}

int chan_free(struct nn_chan *chan)
{
    dpool_free(chan->dpool);
    free(chan->nodes);
    free(chan);
    return 0;
}

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

        if(chan->nodes){
            ICHK(LWARN, r, ll_free(chan->nodes));
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

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(chan->id == id){
        _g = chan;
        done = 1;
    }

    ROUTER_CHAN_ITER_POST(rt);
    router_unlock(rt);

    return _g;
}

int router_add_to_chan(struct nn_router *rt, struct nn_chan *ch, struct nn_node *n)
{
    int r = 1;
    assert(n);

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(chan == ch){
        ICHK(LWARN, r, ll_add_front(chan->nodes, (void **)&n));
        done = 1;
    }

    ROUTER_CHAN_ITER_POST(rt);
    router_unlock(rt);

    return r;
}

int router_rem_from_chan(struct nn_router *rt, int chan_id, struct nn_node *n)
{
    int r = 1;

    router_lock(rt);
    ROUTER_CHAN_ITER_PRE(rt);

    if(chan->id == chan_id){
        ICHK(LWARN, r, ll_rem(chan->nodes, n));
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

static int router_rx_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;
    int times;

    ROUTER_CHAN_ITER_PRE(rt);
    CHAN_NODES_ITER_PRE(chan);

    assert(n);
    /* loop through nodes's and move pkt's to rt */
    for(times=0; times < 256; times++){
        r = node_get_pkt(n, &pkt);
        if(r == NN_RET_NODE_DONE){
            /* node is dead, remove it */
            router_unlock(rt);
            router_rem_node(rt, n);
            router_lock(rt);
        }else if(!r){
            assert(pkt);

            //rt->nodes_pkts_no--;

            ICHK(LWARN, r, que_add(rt->rx_pkts, pkt));
            rt->rx_pkts_no++;
            rt->rx_pkts_total++;
            router_cond_broadcast(rt);

            pkt_set_state(pkt, PKT_STATE_RT_RX);
        }else{
            break;
        }
    }

    CHAN_NODES_ITER_POST(chan);
    ROUTER_CHAN_ITER_POST(rt);

    return 0;
}


/* rt have to be locked */
static int router_route_pkts(struct nn_router *rt)
{
    int r;
    struct nn_pkt *pkt;
    struct nn_chan *dest_chan;
    int dest_no;
    int c;
    int send_to;
    int pick_up = 1024;

    //L(LDEBUG, "router_route_pkts");
    /* pick pkt's up from router and move to nodes */
    //while(--pick_up && !router_get_rx_pkt(rt, &pkt)){
    while(--pick_up && !router_get_rx_pkt(rt, &pkt)){
        assert(pkt);

        //if(pkt_cancelled(pkt)){
        //    continue;
        //}

        dest_chan = pkt_get_dest_chan(pkt);
        dest_no = pkt_get_dest_no(pkt);
        c = 0;
        send_to = -1;

        ROUTER_CHAN_ITER_PRE(rt);

        /* route to the dest group */
        if(chan == dest_chan){

            send_to = 0;
            CHAN_NODES_ITER_PRE(chan);

            /* don't send to sender */
            if(n != pkt_get_src(pkt)){
                pkt_inc_refcnt(pkt, 1);
                r = node_put_pkt(n, pkt);
                if(r == NN_RET_NODE_DONE){
                    /* node is dead, remove it */
                    router_unlock(rt);
                    router_rem_node(rt, n);
                    router_lock(rt);
                }else if(!r){
                    send_to++;
                    if(dest_no && ++c >= dest_no){
                        done = 1;
                    }

                }else{
                    pkt_inc_refcnt(pkt, -1);
                }

            }

            CHAN_NODES_ITER_POST(chan);

            rt->tx_pkts_total++;

            /* found and routed to dest group so done */
            done = 1;
        }

        ROUTER_CHAN_ITER_POST(rt);

        pkt_set_state(pkt, PKT_STATE_RT_ROUTED);
        pkt_free(pkt);

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

static struct router_nodes_iter *router_nodes_iter_init(struct nn_router *rt)
{
    return (struct router_nodes_iter *)ll_iter_init(rt->nodes);
}

static int router_nodes_iter_free(struct router_nodes_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_nodes_iter_next(struct router_nodes_iter *iter, struct nn_node **n)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)n);
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

static struct chan_nodes_iter *chan_nodes_iter_init(struct nn_chan *chan)
{
    return (struct chan_nodes_iter *)ll_iter_init(chan->nodes);
}

static int chan_nodes_iter_free(struct chan_nodes_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int chan_nodes_iter_next(struct chan_nodes_iter *iter, struct nn_node **n)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)n);
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

    printf("router->nodes\t");
    printf("rt=%p, cn:%p, n=%p\n", rt, cn, _nodes_get_node(cn));

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

