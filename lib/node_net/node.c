
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
#include "_conn.h"

#include "node.h"
#include "node_drivers/node_driver.h"


struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    struct que *tx_pkts;   /* pkt's going out */

    /* rel */
    struct ll *conn;      /* all conn connected to via conn's */

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

/* passed when thread is started */
struct node_pdata {
    void *(*user_func)(struct nn_node *n, void *pdata);
    struct nn_node *n;
    void *pdata;
};


static void *node_thread(void *arg);
static int node_conn_free_all(struct nn_node *n);
static void *start_user_thread(void *arg);
int node_print(struct nn_node *n);;

static int node_lock(struct nn_node *n);
static int node_unlock(struct nn_node *n);
static int node_cond_wait(struct nn_node *n);
static int node_cond_broadcast(struct nn_node *n);

static int node_tx_pkts(struct nn_node *n);
static int node_rx_pkts(struct nn_node *n);

/* iter */
static struct node_conn_iter *node_conn_iter_init(struct nn_node *rt);
static int node_conn_iter_free(struct node_conn_iter *iter);
static int node_conn_iter_next(struct node_conn_iter *iter, struct nn_conn **cn);


static int node_isok(struct nn_node *n);

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
    while(!done && !node_conn_iter_next(iter, &cn)){

#define NODE_CONN_ITER_POST \
    } \
    node_conn_iter_free(iter); \
    node_unlock(n); \
    }

/* main thread rx input from conn, call user functions, tx output to conn  */
static void *node_thread(void *arg)
{
    struct nn_node *n = arg;
    void *(*user_func)(struct nn_node *n, void *pdata);
    void *pdata;
    int attr;
    //struct node_driver_ops *ops;
    thread_t tid;
    int r;
    enum nn_state state;

    user_func = node_get_codep(n);
    pdata = node_get_pdatap(n);
    attr = node_get_attr(n);

    //ops = node_driver_get_ops(node_get_type(n));
    L(LNOTICE, "Node thread starting: %p", n);

    if(user_func){
        struct node_pdata *arg;
        PCHK(LWARN, arg, malloc(sizeof(*arg)));
        if(!arg){
            goto err;
        }
        arg->user_func = user_func;
        arg->n = n;
        arg->pdata = pdata;
        thread_create(&tid, NULL, start_user_thread, arg);
        //thread_detach(tid);
    }

    for(;;){

        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            thread_join(tid, NULL);
            node_conn_free_all(n);
            node_set_state(n, NN_STATE_FINISHED);
            return NULL;
        }

        ICHK(LDEBUG, r, node_tx_pkts(n));

        ICHK(LDEBUG, r, node_rx_pkts(n));

        //sched_yield();
        usleep(100000);
    }

err:
    L(LNOTICE, "Node thread ended: %p", n);
    return NULL;
}


struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    int r;
    struct nn_node *n;
    thread_t tid;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }
    DBG_STRUCT_INIT(n);

    //n->ops = node_driver_get_ops(n->type);
    //assert(n->ops);

    PCHK(LWARN, n->conn, ll_init());
    if(!n->conn){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    //PCHK(LWARN, n->grp_conns, ll_init());
    //if(!n->grp_conns){
    //    PCHK(LWARN, r, node_free(n));
    //    goto err;
    //}

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

    /* start the node thread that will handle coms from and to node */
    thread_create(&tid, NULL, node_thread, n);
    thread_detach(tid);

err:
    return n;
}

int node_free(struct nn_node *n)
{
    int fail = 0;
    int r = 0;
    struct timespec ts = {0, 0};

    struct nn_pkt *pkt;

    node_isok(n);

    mutex_lock(&n->mutex);

    if(n->conn){
        ICHK(LWARN, r, ll_free(n->conn));
        if(r) fail++;
    }

    //if(n->grp_conns){
    //    ICHK(LWARN, r, ll_free(n->grp_conns));
    //    if(r) fail++;
    //}

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

    node_lock(n);

    ICHK(LWARN, r, ll_add_front(n->conn, (void **)&cn));
    if(r) goto err;

    ICHK(LWARN, r, _conn_set_node(cn, n));
    if(r) goto err;

err:
    node_unlock(n);
    return r;
}

int node_unconn(struct nn_node *n, struct nn_conn *cn)
{
    int r;

    node_isok(n);

    node_lock(n);

    ICHK(LWARN, r, ll_rem(n->conn, cn));
    if(r) goto err;

    ICHK(LWARN, r, _conn_free_node(cn));
    if(r) goto err;

err:
    node_unlock(n);
    return r;
}

int node_set_rx_cnt(struct nn_node *n, int grp_id, int cnt)
{
    NODE_CONN_ITER_PRE

    if(_conn_get_node(cn) == n){
        _conn_set_rx_cnt(cn, grp_id, cnt);
        done = 1;
    }

    NODE_CONN_ITER_POST

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

struct nn_conn *node_get_router_conn(struct nn_node *n, struct nn_router *rt)
{
    struct nn_conn *_cn = NULL;

    NODE_CONN_ITER_PRE
    if(_conn_get_router(cn) == rt){
        done = 1;
        _cn = cn;
    }

    NODE_CONN_ITER_POST

    return _cn;
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

int node_tx(struct nn_node *n, struct nn_pkt *pkt)
{
    int r = 1;

    node_lock(n);

    ICHK(LWARN, r, que_add(n->tx_pkts, pkt));

    node_unlock(n);

    L(LDEBUG, "+ node_tx %p(%d)\n", pkt, r);

    return r;
}

#if 0
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
#endif

/* helper functions */

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

static int node_tx_pkts(struct nn_node *n)
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

        /* just send to first one */
        _conn_node_tx_pkt(cn, pkt);
        //while(_conn_node_tx_pkt(cn, pkt)){
        //    usleep(10000);
        //}
        done = 1;
        NODE_CONN_ITER_POST
    }

end:
    return 0;
}

static int node_rx_pkts(struct nn_node *n)
{
    int r = 0;
    struct nn_pkt *pkt;

    NODE_CONN_ITER_PRE

    /* pick pkt's up from conn and move to node */
    if(!_conn_node_rx_pkt(cn, &pkt)){
        assert(pkt);

        ICHK(LWARN, r, que_add(n->rx_pkts, pkt));
        L(LDEBUG, "+ node_rx_pkts %p(%d)", pkt, r);
    }

    NODE_CONN_ITER_POST

    return r;
}

/* free all node sides of all n->conn's and g->... */
static int node_conn_free_all(struct nn_node *n)
{
    int r = 0;
    struct node_conn_iter *iter;
    struct nn_conn *cn;

    iter = node_conn_iter_init(n);
    /* remove the conns to routers */
    while(!node_conn_iter_next(iter, &cn)){
        r = _conn_free_node(cn);
    }
    node_conn_iter_free(iter);

    return r;

}

static void *start_user_thread(void *arg)
{
    struct node_pdata *info = arg;
    void *(*user_func)(struct nn_node *n, void *pdata) = info->user_func;
    struct nn_node *n = info->n;
    void *pdata = info->pdata;

    free(arg);

    return user_func(n, pdata);
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
    printf("n=%p, cn:%p, rt=%p\n", n, cn, _conn_get_router(cn));

    NODE_CONN_ITER_POST

    return 0;
}


/* iter */
static struct node_conn_iter *node_conn_iter_init(struct nn_node *rt)
{
    return (struct node_conn_iter *)ll_iter_init(rt->conn);
}

static int node_conn_iter_free(struct node_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int node_conn_iter_next(struct node_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
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


