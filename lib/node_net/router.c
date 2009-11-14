
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


/* easy iterator pre/post
 * rt != NULL when this is called, afterwards cn for the
 matching each matching node is set and can be used */

#define ROUTER_GRP_ITER_PRE \
    { \
    assert(rt); \
    int done = 0; \
    struct router_grp_iter *iter = NULL; \
    struct nn_grp *g; \
    router_lock(rt); \
    iter = router_grp_iter_init(rt); \
    while(!done && !router_grp_iter_next(iter, &g)){ \

#define ROUTER_GRP_ITER_POST \
    } \
    router_grp_iter_free(iter); \
    router_unlock(rt); \
    }

#define GRP_CONN_ITER_PRE \
    { \
    assert(g); \
    int done = 0; \
    struct grp_conn_iter *iter = NULL; \
    struct nn_conn *cn; \
    iter = grp_conn_iter_init(g); \
    while(!done && !grp_conn_iter_next(iter, &cn)){ \

#define GRP_CONN_ITER_POST \
    } \
    grp_conn_iter_free(iter); \
    }

        //_conn_lock(cn);
       // _conn_unlock(cn); \

/* each group has it's own list of connections and rx/tx bufs */
struct nn_grp {
    int id;
    struct ll *conn; /* all the conn's */

    struct que *rx_pkts;   /* pkt's coming in */
    struct que *tx_pkts;   /* pkt's going out */

    //mutex_t mutex;
    //cond_t cond;

    //struct que *conn_data_avail; /* conn with data available */
    //io_pkt_req_cb_t io_in_pkt_cb;
    //io_data_req_cb_t io_in_data_cb;

};


struct nn_router{
    struct ll *grp;
    enum nn_state state;

    mutex_t mutex;
    cond_t cond;
};


/* this type don't exist, just for type checking */
struct router_grp_iter;

void *router_thread(void *arg);
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

static struct router_grp_iter *router_grp_iter_init(struct nn_router *rt);
static int router_grp_iter_free(struct router_grp_iter *iter);
static int router_grp_iter_next(struct router_grp_iter *iter, struct nn_grp
        **g);

static struct grp_conn_iter *grp_conn_iter_init(struct nn_grp *g);
static int grp_conn_iter_free(struct grp_conn_iter *iter);
static int grp_conn_iter_next(struct grp_conn_iter *iter, struct nn_conn **cn);

#if 0
static int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn, void *a0), 
        struct nn_pkt *pkt);
#endif

int router_isvalid(struct nn_router *rt)
{
    //assert(rt->conn);
    return 0;
}


struct nn_router *router_init()
{
    struct nn_router *rt;
    thread_t tid;

    PCHK(LWARN, rt, calloc(1, sizeof(*rt)));
    if(!rt){
        goto err;
    }

    PCHK(LWARN, rt->grp, ll_init());
    if(!rt->grp){
        router_free(rt);
        rt = NULL;
        goto err;
    }

    // FIXME: err checking
    mutex_init(&rt->mutex, NULL);
    cond_init(&rt->cond, NULL);

    rt->state = NN_STATE_PAUSED;

    router_isvalid(rt);

    /* start router_thread */
    thread_create(&tid, NULL, router_thread, rt);

err:
    return rt;
}


int router_free(struct nn_router *rt)
{
    int r = 0;

    router_isvalid(rt);

    mutex_lock(&rt->mutex);

    if(rt->grp){

        ICHK(LWARN, r, ll_free(rt->grp));
    }


    mutex_unlock(&rt->mutex);

    cond_destroy(&rt->cond);
    mutex_destroy(&rt->mutex);

    free(rt);

    return r;
}

int router_add_grp(struct nn_router *rt, int id)
{
    int r = 1;
    struct nn_grp *g;

    mutex_lock(&rt->mutex);

    PCHK(LWARN, g, calloc(1, sizeof(*g)));
    if(!g){
        goto err;
    }
    g->id = id;

    PCHK(LWARN, g->conn, ll_init());
    if(!g->conn){
        router_rem_grp(rt, id);
        g = NULL;
        goto err;
    }

    PCHK(LWARN, g->rx_pkts, que_init(100));
    if(!(g->rx_pkts)){
        PCHK(LWARN, r, router_rem_grp(rt, id));
        goto err;
    }

    PCHK(LWARN, g->tx_pkts, que_init(100));
    if(!(g->tx_pkts)){
        PCHK(LWARN, r, router_rem_grp(rt, id));
        goto err;
    }

    /* add newly created grp */
    ICHK(LWARN, r, ll_add_front(rt->grp, (void **)&g));
    if(r){
        PCHK(LWARN, r, router_rem_grp(rt, id));
        goto err;
    }

    r = 0;

err:
    mutex_unlock(&rt->mutex);

    return r;
}


/* return -1 no such group */
int router_rem_grp(struct nn_router *rt, int id)
{
    int r;
    int fail = -1;
    struct nn_pkt *pkt;
    struct timespec ts = {0, 0};

    ROUTER_GRP_ITER_PRE

    if(g->id == id){

        done = 1;
        fail = 0;

        if(g->conn){
            ICHK(LWARN, r, ll_free(g->conn));
        }

        if(g->rx_pkts){
            while((pkt=que_get(g->rx_pkts, &ts))){
                pkt_free(pkt);
            }
            ICHK(LWARN, r, que_free(g->rx_pkts));
            if(r) fail++;
        }

        if(g->tx_pkts){
            while((pkt=que_get(g->tx_pkts, &ts))){
                pkt_free(pkt);
            }
            ICHK(LWARN, r, que_free(g->tx_pkts));
            if(r) fail++;
        }

        if(!fail){
            ICHK(LWARN, r, ll_rem(rt->grp, g));
            if(r) fail++;
        }

        free(g);
    }

    ROUTER_GRP_ITER_POST

    return fail;
}

struct nn_grp *router_get_grp(struct nn_router *rt, int id)
{
    struct nn_grp *_g = NULL;

    ROUTER_GRP_ITER_PRE

    if(g->id == id){
        _g = g;
        done = 1;
    }

    ROUTER_GRP_ITER_POST

    return _g;
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


int router_conn(struct nn_router *rt, int grp_id, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    ROUTER_GRP_ITER_PRE

    if(g->id == grp_id){

        fail = 0;

        _conn_lock(cn);

        ICHK(LWARN, r, ll_add_front(g->conn, (void **)&cn));
        if(r) fail++;

        ICHK(LWARN, r, _conn_set_router(cn, g));
        if(r) fail++;

        _conn_unlock(cn);

        done = 1;
    }

    ROUTER_GRP_ITER_POST

    _conn_unlock(cn);
    router_unlock(rt);

    return r;
}

int router_unconn(struct nn_router *rt, int grp_id, struct nn_conn *cn)
{
    int r;
    int fail = -1;

    router_isvalid(rt);

    ROUTER_GRP_ITER_PRE

    if(g->id == grp_id){
        fail = 0;

        _conn_lock(cn);

        ICHK(LWARN, r, ll_rem(g->conn, cn));
        if(r) fail++;

        r = _conn_free_router(cn);

        _conn_unlock(cn);

        done = 1;
    }


    ROUTER_GRP_ITER_POST

    return r;
}

#if 0


int router_print(struct nn_router *rt)
{

    router_grp_iter_PRE

    printf("router->conn\t");
    printf("rt=%p, cn:%p, n=%p\n", rt, cn, _conn_get_node(cn));

    router_grp_iter_POST

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

#endif


/* pick up pkts coming to router, and router to other node's, main loop */
void *router_thread(void *arg)
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

        /* rx packets from conn */
        //router_rx_pkts(rt);

        /* tx packet to conn */
        //router_tx_pkts(rt);

        sched_yield();

    }
    return NULL;
}


/* free router side of conn */
static int router_conn_free_all(struct nn_router *rt)
{
    int fail = 0;
    int r;
    //struct router_grp_iter *iter;
    //struct nn_conn *cn;

    ROUTER_GRP_ITER_PRE
    GRP_CONN_ITER_PRE

    //r = router_unconn(rt, g->id, cn);

    ICHK(LWARN, r, ll_rem(g->conn, cn));
    if(r) fail++;

    r = _conn_free_router(cn);

    GRP_CONN_ITER_POST

    router_unlock(rt);
    router_rem_grp(rt, g->id);
    router_lock(rt);

    ROUTER_GRP_ITER_POST


/*
    iter = router_grp_iter_init(rt);
    while(!router_grp_iter_next(iter, &cn)){
        r = router_unconn(rt, cn);
    }
    router_grp_iter_free(iter);
*/
    return fail;
}

#if 0

static int router_tx_pkts(struct nn_router *rt)
{
    struct nn_pkt *pkt;
    struct nn_grp *g;
    int dest_no;
    int c;
    int send_to;

    /* pick pkt's up from router and move to conn */
    while(!router_get_rx_pkt(rt, &pkt)){
        assert(pkt);

        g = pkt_get_dest(pkt);
        dest_no = pkt_get_dest_no(pkt);
        c = 0;
        send_to = 0;

        /* ROUTING: IMPROVE:!! */

        router_grp_iter_PRE


/*
        if(!grp_is_memb(g, _conn_get_node(cn))){
            pkt_inc_refcnt(pkt, 1);
            while(_conn_router_tx_pkt(cn, pkt)){
                _conn_unlock(cn);
                usleep(10000);
                _conn_lock(cn);
            }
            send_to++;
            if(dest_no && ++c >= dest_no){
                done = 1;
            }
        }
*/

        router_grp_iter_POST

        if(!send_to){
            /* not send, add back */
            router_add_rx_pkt(rt, &pkt);
        }else{
            /* free for the orignal packet */
            pkt_free(pkt);
        }

    }

    return 0;
}

static int router_rx_pkts(struct nn_router *rt)
{
    struct nn_pkt *pkt;

    router_grp_iter_PRE

    /* pick pkt's up from router and move to conn */
    if(!_conn_router_rx_pkt(cn, &pkt)){
        assert(pkt);

        _conn_unlock(cn);
        router_unlock(rt);
        // FIXME: sending to all routers
        //router_grp_iter_PRE
        router_add_rx_pkt(rt, pkt);

        router_lock(rt);
        _conn_lock(cn);
        //router_grp_iter_POST
    }

    router_grp_iter_POST

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
#endif

static struct router_grp_iter *router_grp_iter_init(struct nn_router *rt)
{
    return (struct router_grp_iter *)ll_iter_init(rt->grp);
}

static int router_grp_iter_free(struct router_grp_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int router_grp_iter_next(struct router_grp_iter *iter, struct nn_grp **g)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)g);
}

static struct grp_conn_iter *grp_conn_iter_init(struct nn_grp *g)
{
    return (struct grp_conn_iter *)ll_iter_init(g->conn);
}

static int grp_conn_iter_free(struct grp_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int grp_conn_iter_next(struct grp_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}

#if 0
static int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn, void *a0),
        struct nn_pkt *pkt)
{
    int r;
    assert(rt);
    struct router_grp_iter *iter;
    struct nn_conn *cn;

    router_lock(rt);

    iter = router_grp_iter_init(rt);
    while(!router_grp_iter_next(iter, &cn)){
        _conn_lock(cn);
        r = cb(cn, pkt);
        _conn_unlock(cn);
        if(r){
            break;
        }
    }
    router_grp_iter_free(iter);

    router_unlock(rt);

    return r;

}
#endif

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
