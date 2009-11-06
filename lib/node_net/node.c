
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"
#include "util/dbg.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "pkt.h"
#include "conn.h"

#include "node.h"
#include "node_io.h"
#include "node_drivers/node_driver.h"


/* easy iterator pre/post
 * n != NULL when this is called, each iteration cn for
 each matching router is locked and can be used */
#define NODE_CONN_ITER_PRE \
    { \
    assert(n); \
    int done = 0; \
    struct node_conn_iter *iter; \
    struct nn_conn *cn; \
    node_lock(n); \
    iter = node_conn_iter_init(n); \
    while(!done && !node_conn_iter_next(iter, &cn)){ \
        conn_lock(cn);

#define NODE_CONN_ITER_POST \
        conn_unlock(cn); \
    } \
    node_conn_iter_free(iter); \
    node_unlock(n); \
    }


static int node_isok(struct nn_node *n);

struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    struct que *tx_pkts;   /* pkt's going out */

    /* rel */
    struct ll *conn;      /* all conn connected to via conn's */
    struct ll *grp_conns;     /* all groups connected to via conn's */

    /* funcp's to communicate with this node type driver */
    struct node_driver_ops *ops;

    void *code;  /* pointer to object depending on type */

    void *pdata; /* private passthru data */

    /* maximum time a node exe is allowed to run on one buffer processing */
    uint32_t max_exe_usec;

    mutex_t mutex;
    cond_t cond;

    DBG_STRUCT_END
};


/* for nn_node->grp_conns ll */
struct nn_node_grp {
    struct nn_grp *grp;
};


/** node type **/

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    int r;
    struct nn_node *n;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }
    DBG_STRUCT_INIT(n);

    //n->ops = node_driver_get_ops(n->type);
    //assert(n->ops);

    PCHK(LWARN, n->grp_conns, ll_init());
    if(!n->grp_conns){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->conn, ll_init());
    if(!n->conn){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->rx_pkts, que_init(100));
    if(!(n->rx_pkts)){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->tx_pkts, que_init(100));
    if(!(n->tx_pkts)){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    // FIXME: err checking
    mutex_init(&n->mutex, NULL);
    cond_init(&n->cond, NULL);

    n->state = NN_STATE_PAUSED;

    node_isok(n);
    ICHK(LWARN, r, node_io_run(n));

err:
    return n;
}

static int node_lock(struct nn_node *n)
{
    mutex_lock(&n->mutex);
    return 0;
}

static int node_unlock(struct nn_node *n)
{
    mutex_unlock(&n->mutex);
    return 0;
}

static int node_cond_wait(struct nn_node *n)
{
    cond_wait(&n->cond, &n->mutex);
    return 0;
}

static int node_cond_broadcast(struct nn_node *n)
{
    cond_broadcast(&n->cond);
    return 0;
}

int node_free(struct nn_node *n)
{
    struct ll_iter *iter;
    int fail = 0;
    int r = 0;
    struct timespec ts = {0, 0};

    struct nn_node_grp *ng;
    struct nn_pkt *pkt;

    node_isok(n);

    mutex_lock(&n->mutex);

    if(n->conn){
        /*
        iter = ll_iter_init(n->conn);

        while(!ll_iter_next(iter, (void **)&conn)){
            ICHK(LWARN, r, ll_rem(n->conn, ng));
            free(ng);
        }
        ICHK(LWARN, r, ll_free(n->conn));
        if(r) fail++;

        ll_iter_free(iter);
        */

        printf("!! freeing n->conn %p\n", n->conn);
        ICHK(LWARN, r, ll_free(n->conn));
        if(r) fail++;
    }

    if(n->grp_conns){
        iter = ll_iter_init(n->grp_conns);

        while(!ll_iter_next(iter, (void **)&ng)){
            ICHK(LWARN, r, ll_rem(n->grp_conns, ng));
            free(ng);
        }
        ICHK(LWARN, r, ll_free(n->grp_conns));
        if(r) fail++;

        ll_iter_free(iter);

    }

    if(n->rx_pkts){
        while((pkt=que_get(n->rx_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(n->rx_pkts));
        if(r) fail++;
    }

    if(n->tx_pkts){
        while((pkt=que_get(n->tx_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(n->tx_pkts));
        if(r) fail++;
    }

    mutex_unlock(&n->mutex);

    cond_destroy(&n->cond);
    mutex_destroy(&n->mutex);

    free(n);

    return fail;
}

int node_clean(struct nn_node *n)
{
    node_lock(n);

    while(node_get_state(n) != NN_STATE_FINISHED){
        node_cond_wait(n);
    }

    node_unlock(n);

    node_free(n);

    return 0;
}




int node_conn(struct nn_node *n, struct nn_conn *cn)
{
    int r = 1;

    ICHK(LWARN, r, ll_add_front(n->conn, (void **)&cn));
    if(r) goto err;

    ICHK(LWARN, r, conn_set_node(cn, n));
    if(r) goto err;

err:
    return r;
}

int node_unconn(struct nn_node *n, struct nn_conn *cn)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->conn, cn));
    if(r) goto err;

    ICHK(LWARN, r, conn_free_node(cn));
    if(r) goto err;

err:
    return r;
}


int node_join_grp(struct nn_node *n, struct nn_grp *g)
{
    int r = 1;
    struct nn_node_grp *eg;

    PCHK(LWARN, eg, malloc(sizeof(*eg)));
    if(!eg){
        goto err;
    }

    eg->grp = g;
    ICHK(LWARN, r, ll_add_front(n->grp_conns, (void **)&eg));

err:
    return r;
}

int node_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->grp_conns, g));
    return 0;
}


int node_get_type(struct nn_node *n)
{
    return n->type;
}

int node_get_attr(struct nn_node *n)
{
    return n->attr;
}

void *node_get_pdatap(struct nn_node *n)
{
    return n->pdata;
}

void *node_get_codep(struct nn_node *n)
{
    return n->code;
}

int node_tx_pkts(struct nn_node *n)
{
    struct nn_pkt *pkt;
    struct timespec ts = {0, 0};

    /* pick pkt's up from node and move to conn */
    for(;;){

        node_lock(n);
        pkt = que_get(n->tx_pkts, &ts);
        node_unlock(n);

        if(pkt){
            L(LDEBUG, "+ got node tx_pkt %p", pkt);
        }else{
            goto end;
        }
        assert(pkt);

        // FIXME: sending to all routers
        NODE_CONN_ITER_PRE
        while(conn_node_tx_pkt(cn, pkt)){
            conn_unlock(cn);
            usleep(10000);
            conn_lock(cn);
        }
        NODE_CONN_ITER_POST
    }

end:
    return 0;
}


int node_rx_pkts(struct nn_node *n)
{
    int r = 0;
    struct nn_pkt *pkt;

    NODE_CONN_ITER_PRE

    /* pick pkt's up from conn and move to node */
    if(!conn_node_rx_pkt(cn, &pkt)){
        assert(pkt);

        // FIXME: sending to all nodes
        ICHK(LWARN, r, que_add(n->rx_pkts, pkt));
        L(LDEBUG, "+ node_add_rx_pkt %p(%d)", pkt, r);
    }

    NODE_CONN_ITER_POST

    return r;
}

int node_get_rx_pkt(struct nn_node *n, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    node_lock(n);

    *pkt = que_get(n->rx_pkts, &ts);

    node_unlock(n);

    if(*pkt){
        L(LDEBUG, "+ node_get_rx_pkt %p", *pkt);
        r = 0;
    }

    return r;
}

struct nn_conn *node_get_router_conn(struct nn_node *n, struct nn_router *rt)
{
    struct nn_conn *_cn = NULL;

    NODE_CONN_ITER_PRE
    if(conn_get_router(cn) == rt){
        done = 1;
        _cn = cn;
    }

    NODE_CONN_ITER_POST

    return _cn;
}


int node_set_state(struct nn_node *n, enum nn_state state)
{
    node_lock(n);

    n->state = state;

    node_cond_broadcast(n);
    node_unlock(n);

    return 0;
}

enum nn_state node_get_state(struct nn_node *n)
{
    return n->state;
}

int node_do_state(struct nn_node *n)
{
    enum nn_state state;

    node_lock(n);

    switch((state=node_get_state(n))){
        case NN_STATE_RUNNING:
            break;
        case NN_STATE_PAUSED:
            L(LNOTICE, "Node paused: %p", n);
            while(node_get_state(n) == NN_STATE_PAUSED){
                node_unlock(n);
                sleep(1);
                node_lock(n);
                //node_cond_wait(n);
            }
            L(LNOTICE, "Node paused state exit: %p", n);
            break;
        case NN_STATE_SHUTDOWN:
            L(LNOTICE, "Node thread shutdown start: %p", n);
            node_unlock(n);
            L(LNOTICE, "Node thread shutdown completed");
            break;
        case NN_STATE_FINISHED:
            L(LCRIT, "Node thread illegally in finished state");
            break;
    }

    node_unlock(n);

    return state;
}

/* iter */

struct node_conn_iter *node_conn_iter_init(struct nn_node *rt)
{
    return (struct node_conn_iter *)ll_iter_init(rt->conn);
}

int node_conn_iter_free(struct node_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

int node_conn_iter_next(struct node_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}



/** debug functions **/

static int node_isok(struct nn_node *n)
{

    DBG_STRUCT_ISOK(n);

    assert(n->type >= 0 && n->type < 128);
    assert(n->attr >= 0 && n->attr < 128);
    assert(n->code);

    //node_router_isok(n);
    //node_grp_isok(n);
    //node_print(n);

    return 0;
}

int node_print(struct nn_node *n)
{
    int c;
    //void *iter;

    c = 0;

    NODE_CONN_ITER_PRE

    printf("node->conn\t");
    printf("n=%p, cn:%p, rt=%p\n", n, cn, conn_get_router(cn));

    NODE_CONN_ITER_POST

    return 0;
}

int node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt)
{
    int r;

    assert(pkt);

    node_lock(n);

    ICHK(LWARN, r, que_add(n->tx_pkts, pkt));

    node_unlock(n);

    L(LDEBUG, "+ node_add_tx_pkt %p(%d)\n", pkt, r);

    return r;
}

#if 0
int node_get_tx_pkt(struct nn_node *n, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    node_lock(n);

    *pkt = que_get(n->tx_pkts, &ts);

    node_unlock(n);

    if(*pkt){
        L(LDEBUG, "+ node_get_tx_pkt %p", *pkt);
        r = 0;
    }

    return r;
}
#endif

/*
int node_add_rx_pkt(struct nn_node *n, struct nn_pkt *pkt)
{
    int r;

    //node_lock(n);

    ICHK(LWARN, r, que_add(n->rx_pkts, pkt));

    //node_unlock(n);

    L(LDEBUG, "+ node_add_rx_pkt %p(%d)", pkt, r);

    return r;
}
*/


#if 0
/* check that the conn's it points to points back */
static int node_router_isok(struct nn_node *n)
{
    int r = 0;
    void *iter;

    iter = NULL;
   // while((cn=ll_iter_next(n->conn, &iter))){
   //     /* make sure we are a member */
   //     //r = router_isconn(rt->conn, n);
   //     //r = router_print(rt->conn);
   //     //assert(r == 0);
   //     //if(r){
   //     //    break;
   //     //}
   // }

    return r;
}

/* check that the grp's it points to points back */
static int node_grp_isok(struct nn_node *n)
{
    int r = 0;
    void *iter;

    iter = NULL;
   // while((g=ll_iter_next(n->grp_conns, &iter))){
   //     r = grp_ismemb(g->grp, n);
   //     assert(r == 0);
   //     //r = grp_print(g->grp);
   //     if(r){
   //         break;
   //     }
   // }

    return r;
}

#endif


/** debug functions ends **/


