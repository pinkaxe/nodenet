
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

#include "node.h"
#include "node_drivers/node_driver.h"


struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;
    enum nn_state state;

    struct que *rx_pkts;   /* pkt's coming in */
    int rx_pkts_no;
    struct que *tx_pkts;   /* pkt's going out */
    int tx_pkts_no;

    int rx_pkts_total;
    int tx_pkts_total;

    /* rel */
    //struct ll *conn;      /* all conn connected to via conn's */

    void *code;  /* pointer to object depending on type */

    void *pdata; /* private passthru data */

    /* maximum time a node exe is allowed to run on one buffer processing */
    uint32_t max_exe_usec;

    mutex_t mutex;
    cond_t cond;

    int refcnt;

    DBG_STRUCT_END
};

/* passed when thread is started */
struct node_pdata {
    void *(*user_func)(struct nn_node *n, void *pdata);
    struct nn_node *n;
    void *pdata;
};


//static void *node_thread(void *arg);

//static void *node_thread(void *arg);
//static int node_conn_free_all(struct nn_node *n);
static void *start_user_thread(void *arg);
int node_print(struct nn_node *n);;

//static int node_get_type(struct nn_node *n);
static int node_get_attr(struct nn_node *n);
static void *node_get_codep(struct nn_node *n);

static int node_lock(struct nn_node *n);
static int node_unlock(struct nn_node *n);
static int node_cond_wait(struct nn_node *n);
static int node_cond_broadcast(struct nn_node *n);

static int node_isok(struct nn_node *n);


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

    PCHK(LWARN, n->rx_pkts, que_init(999));
    if(!(n->rx_pkts)){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }
    n->rx_pkts_no = 0;
    n->rx_pkts_total = 0;

    PCHK(LWARN, n->tx_pkts, que_init(999));
    if(!(n->tx_pkts)){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }
    n->tx_pkts_no = 0;
    n->tx_pkts_total = 0;
    n->refcnt = 0;

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    // FIXME: err checking
    mutex_init(&n->mutex, NULL);
    cond_init(&n->cond, NULL);

    n->state = NN_STATE_PAUSED;

    //node_isok(n);

    /* start the node thread that will handle coms from and to node */
    //thread_create(&tid, NULL, node_thread, n);
    //thread_detach(tid);

    if(code){
        thread_t tid;
        struct node_pdata *arg;
        void *(*user_func)(struct nn_node *n, void *pdata);

        user_func = code;

        PCHK(LWARN, arg, malloc(sizeof(*arg)));
        if(!arg){
            goto err;
        }
        arg->user_func = user_func;
        arg->n = n;
        arg->pdata = pdata;
        thread_create(&tid, NULL, start_user_thread, arg);
        thread_detach(tid);
    }

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

    node_lock(n);

    if(n->rx_pkts){
        //printf("!!! n->rx_pkts: %p\n", n->rx_pkts);
        while((pkt=que_get(n->rx_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(n->rx_pkts));
        if(r) fail++;

        n->rx_pkts = NULL;
    }

    if(n->tx_pkts){
        while((pkt=que_get(n->tx_pkts, &ts))){
            //printf("!!! packet: %p\n", pkt);
        //    abort();
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(n->tx_pkts));
        if(r) fail++;
        n->tx_pkts = NULL;
    }

    node_unlock(n);

    cond_destroy(&n->cond);
    mutex_destroy(&n->mutex);

    free(n);

    return fail;
}

int node_clean(struct nn_node *n)
{
    node_lock(n);

    while(node_get_state(n) != NN_STATE_DONE){
        node_cond_wait(n);
    }

    node_unlock(n);

    node_free(n);

    return 0;
}


int node_wait(struct nn_node *n)
{
    node_lock(n);

    while(node_get_state(n) == NN_STATE_RUNNING && n->rx_pkts_no <= 0){
        node_cond_wait(n);
    }

    node_unlock(n);

    return 0;
}


void *node_get_pdatap(struct nn_node *n)
{
    return n->pdata;
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
            break;
        case NN_STATE_DONE:
            L(LCRIT, "Node thread illegally in finished state");
            break;
    }

    node_unlock(n);

    return state;
}

int node_inc_refcnt(struct nn_node *n, int cnt)
{
    int r;

    node_lock(n);

    n->refcnt += cnt;
    r = n->refcnt;

    node_unlock(n);

    return r;
}

int node_put_pkt(struct nn_node *n, struct nn_pkt *pkt)
{
    int r;

    node_lock(n);

    if(n->state == NN_STATE_FINISHED && n->refcnt <= 0){
        node_unlock(n);
        node_free(n);
        r = NN_RET_NODE_DONE;
        goto done;
    }

    ICHK(LWARN, r, que_add(n->rx_pkts, pkt));
    n->rx_pkts_no++;
    n->rx_pkts_total++;
    node_cond_broadcast(n);
    L(LDEBUG, "+ node_rx_pkts %p(%d)", pkt, r);

    node_unlock(n);

done:

    return r;
}


int node_check(struct nn_node *n)
{
    int r = 0;

    node_lock(n);

    if(n->state == NN_STATE_FINISHED && n->refcnt <= 0){
        node_unlock(n);
        node_free(n);
        r = NN_RET_NODE_DONE;
        goto done;
    }

    node_unlock(n);

done:
    return r;
}


int node_get_pkt(struct nn_node *n, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    node_lock(n);

    if(n->state == NN_STATE_FINISHED && n->refcnt <= 0){
        node_unlock(n);
        node_free(n);
        r = NN_RET_NODE_DONE;
        goto done;
    }

    *pkt = que_get(n->tx_pkts, &ts);
    n->tx_pkts_no--;

    node_unlock(n);

    if(*pkt){// && !pkt_cancelled(*pkt)){
        pkt_set_state(*pkt, PKT_STATE_N_RX);
        L(LDEBUG, "+ node_get_pkt %p", *pkt);
        r = 0;
    }

done:
    return r;
}


int node_rx(struct nn_node *n, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    node_lock(n);

    *pkt = que_get(n->rx_pkts, &ts);
    n->rx_pkts_no--;

    node_unlock(n);

    if(*pkt){// && !pkt_cancelled(*pkt)){
        pkt_set_state(*pkt, PKT_STATE_N_RX);
        L(LDEBUG, "+ node_rx %p", *pkt);
        r = 0;
    }

    return r;
}

int node_tx(struct nn_node *n, struct nn_pkt *pkt)
{
    int r = 1;

    node_lock(n);

    ICHK(LWARN, r, que_add(n->tx_pkts, pkt));

    n->tx_pkts_no++;
    n->tx_pkts_total++;
    node_cond_broadcast(n);

    node_unlock(n);

    pkt_set_state(pkt, PKT_STATE_N_TX);

    L(LDEBUG, "+ node_tx %p(%d)\n", pkt, r);

    return r;
}

enum nn_state node_get_state(struct nn_node *n)
{
    enum nn_state state;

    state = n->state;

    return state;
}

int node_set_state(struct nn_node *n, enum nn_state state)
{
    node_lock(n);

    if(state == NN_STATE_FINISHED && n->refcnt <= 0){
        /* if it's finished with no routers pointing to it, free */
        node_unlock(n);
        node_free(n);
        n = NULL;
        goto end;
    }

    n->state = state;

    node_cond_broadcast(n);
    node_unlock(n);

end:
    return 0;
}


int node_set_rx_cnt(struct nn_node *n, int grp_id, int cnt)
{
#if 0
    node_lock(n);
    NODE_CONN_ITER_PRE

    if(_conn_get_node(cn) == n){
        _conn_set_rx_cnt(cn, grp_id, cnt);
        done = 1;
    }

    NODE_CONN_ITER_POST
    node_unlock(n);
#endif

    return 0;
}

int node_get_status(struct nn_node *n, struct node_status *status)
{
    node_lock(n);

    status->rx_pkts_no = n->rx_pkts_no;
    status->tx_pkts_no = n->tx_pkts_no;
    status->rx_pkts_total = n->rx_pkts_total;
    status->tx_pkts_total = n->tx_pkts_total;

    node_unlock(n);

    return 0;
}


/*
static int node_get_type(struct nn_node *n)
{
    return n->type;
}
*/

static int node_get_attr(struct nn_node *n)
{
    return n->attr;
}


static void *node_get_codep(struct nn_node *n)
{
    return n->code;
}


int node_rxs_no(struct nn_node *n)
{
    int r;



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

    return 0;
}


/* iter */

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
static int node_isok(struct nn_node *n)
{
    struct nn_node *n1; /* the pointer from the conn */

    NODE_CONN_ITER_PRE(n);

    n1 = _conn_get_node(cn);

    if(n1 != n){
        printf("fuck %p, %p\n", n, n1);
        abort();
    }

    assert(n->type >= 0 && n->type < 128);
    assert(n->attr >= 0 && n->attr < 128);
    assert(n->code);

    NODE_CONN_ITER_POST(n);
    return 0;
}
#endif

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
static int node_chan_isok(struct nn_node *n)
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


