
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"
#include "util/ll.h"
#include "util/link.h"

#include "types.h"
#include "_conn.h"
#include "pkt.h"

/* rename functions for this file */
/* FIXME: do for all *_from *_router for this file */
#define link_free_node link_free_from
#define link_free_router link_free_to

#define link_get_node link_get_from
#define link_get_router link_get_to
#define link_set_node link_set_from
#define link_set_router link_set_to

#define CONN_GRP_PRE(cn) \
    { \
    assert(cn); \
    int done = 0; \
    struct conn_grp_iter *iter = NULL; \
    struct grps *grp; \
    iter = conn_grp_iter_init(cn); \
    while(!done && !conn_grp_iter_next(iter, &grp)){ \

#define CONN_GRP_POST(cn) \
    } \
    conn_grp_iter_free(iter); \
    }

/* one per connected router and node */
struct nn_conn {
    struct link *link; /* keep link info, rt, n, state */
    mutex_t mutex;
    cond_t cond; /* if anything changes */

    struct ll *grps;          /* members of these grp's */

    /* router(output) -> node(input) */
    struct que *rt_n_pkts;   /* router write node pkt */

    /* node(output) -> router(input) */
    struct que *n_rt_pkts;   /* n write data */
    struct que *n_rt_notify; /* always used, from node_driver */
    //struct que *n_rt_pkts;    /* only master node */

};


struct grps {
    int grp_id;
    //struct nn_grp *g; /* router grp it belongs to */
    int rx_cnt; /* counter to only allow a certain amount of rx
                   buffers -2=unlimited(default) */
    enum nn_state state;
};


static int _conn_lock(struct nn_conn *cn);
static int _conn_unlock(struct nn_conn *cn);
static int _conn_dec_rx_cnt(struct nn_conn *cn, struct grps *grp, int dec);

static struct conn_grp_iter *conn_grp_iter_init(struct nn_conn *cn);
static int conn_grp_iter_free(struct conn_grp_iter *iter);
static int conn_grp_iter_next(struct conn_grp_iter *iter, struct
        grps **b);

struct ques_router_init {
    struct que **q;
    size_t size;
};

int ques_init(struct ques_router_init *qs)
{
    int r = 0;
    struct ques_router_init *_qs = qs;

    while((_qs->size > 0)){
        PCHK(LWARN, *(_qs->q), que_init(100));
        if(!*(_qs->q)){
            //PCHK(LWARN, r, _conn_free(cn));
            //goto err;
            // FIXME: free?
            r = 1;
            break;
        }
        _qs++;
    }

    return r;
}

struct nn_conn *_conn_init()
{
    int r = 0;
    struct nn_conn *cn;
    struct link *l;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }

    struct ques_router_init qs[] = {
        {&cn->rt_n_pkts, 100},
        {&cn->n_rt_pkts, 100},
        {&cn->n_rt_notify, 100},
        {NULL, 0},
    };

    r = ques_init(qs);
    if(r){
        _conn_free(cn);
        cn = NULL;
        goto err;
    }
    assert(cn->rt_n_pkts);
    assert(cn->n_rt_pkts);
    assert(cn->n_rt_notify);

    PCHK(LWARN, l, link_init());
    if(!l){
        PCHK(LWARN, r, _conn_free(cn));
        cn = NULL;
        goto err;
    }
    cn->link = l;

    PCHK(LWARN, cn->grps, ll_init());
    if(!cn->grps){
        PCHK(LWARN, r, _conn_free(cn));
        cn = NULL;
        goto err;
    }

    mutex_init(&cn->mutex, NULL);

    link_set_state(cn->link, LINK_STATE_ALIVE);
err:
    return cn;
}

int _conn_free(struct nn_conn *cn)
{
    int r = 0;
    int fail = 0;
    struct timespec ts = {0, 0};
    struct nn_pkt *pkt;

    /* IMPROVE: can save conn state here */
    _conn_lock(cn);

    if(cn->grps){
        /* FIXME: free grp members */
        ICHK(LWARN, r, ll_free(cn->grps));
    }

    /* empty and free the io que's */
    if(cn->rt_n_pkts){
        while((pkt=que_get(cn->rt_n_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(cn->rt_n_pkts));
        if(r) fail++;
    }

    if(cn->n_rt_pkts){
        while((pkt=que_get(cn->n_rt_pkts, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(cn->n_rt_pkts));
        if(r) fail++;
    }

    if(cn->n_rt_notify){
        while((pkt=que_get(cn->n_rt_notify, &ts))){
            pkt_free(pkt);
        }
        ICHK(LWARN, r, que_free(cn->n_rt_notify));
        if(r) fail++;
    }

    _conn_unlock(cn);

    if(&cn->mutex){
        mutex_destroy(&cn->mutex);
    }

    printf("!! freeing cn %p\n", cn);
    free(cn);

    return fail;
}

int _conn_join_grp(struct nn_conn *cn, int grp_id)
{
    int r = 1;
    struct grps *grp;

    PCHK(LWARN, grp, calloc(1, sizeof(*grp)));
    if(!grp){
        goto err;
    }
    grp->grp_id = grp_id;
    grp->rx_cnt = -2;

    ICHK(LWARN, r, ll_add_front(cn->grps, (void **)&grp));
    if(r){
        PCHK(LWARN, r, _conn_quit_grp(cn, grp_id));
        goto err;
    }

    r = 0;
err:
    return r;
}

int _conn_quit_grp(struct nn_conn *cn, int grp_id)
{
    int r = 0;
    //struct timespec ts = {0, 0};
    //struct nn_pkt *pkt;

    _conn_lock(cn);
    CONN_GRP_PRE(cn);

    if(grp->grp_id == grp_id){

        ICHK(LWARN, r, ll_rem(cn->grps, grp));
        free(grp);

        done = 1;
    }

    CONN_GRP_POST(cn);
    _conn_unlock(cn);


    return r;
}

/* returns 1 if cn was free'd */
int _conn_free_node(struct nn_conn *cn)
{
    int r;

    printf("!!! n\n");

    _conn_lock(cn);
    r = link_free_node(cn->link);
    _conn_unlock(cn);

    if(r == 1){
        cn->link = NULL;
        _conn_free(cn);
    }

    return r;
}

int _conn_free_router(struct nn_conn *cn)
{
    int r;

    printf("!!! r\n");

    _conn_lock(cn);
    r = link_free_router(cn->link);
    _conn_unlock(cn);

    if(r == 1){
        cn->link = NULL;
        _conn_free(cn);
    }

    return r;
}


int _conn_set_node(struct nn_conn *cn, struct nn_node *n)
{
    int r;

    _conn_lock(cn);
    r = link_set_from(cn->link, n);
    _conn_unlock(cn);

    return r;
}

int _conn_set_router(struct nn_conn *cn, struct nn_router *rt)
{
    int r;

    _conn_lock(cn);
    r = link_set_router(cn->link, rt);
    _conn_unlock(cn);

    return r;
}

int _conn_set_state(struct nn_conn *cn, int state)
{
    int r;

    _conn_lock(cn);
    r = link_set_state(cn->link, state);
    _conn_unlock(cn);

    return r;
}

struct nn_node *_conn_get_node(struct nn_conn *cn)
{
    struct nn_node *n = NULL;

    _conn_lock(cn);
    n = link_get_node(cn->link);
    _conn_unlock(cn);

    return n;
}

struct nn_router *_conn_get_router(struct nn_conn *cn)
{
    struct nn_router *rt;

    _conn_lock(cn);
    rt = link_get_router(cn->link);
    _conn_unlock(cn);

    return rt;
}

/*
int _conn_get_grp_id(struct nn_conn *cn)
{
    int r;

    _conn_lock(cn);
    r = cn->grp_id;
    _conn_unlock(cn);

    return r;
}
*/

int _conn_get_state(struct nn_conn *cn)
{
    int r;

    _conn_lock(cn);
    r = link_get_state(cn->link);
    _conn_unlock(cn);

    return r;
}


static int _conn_lock(struct nn_conn *cn)
{
    mutex_lock(&cn->mutex);
    return 0;
}

static int _conn_unlock(struct nn_conn *cn)
{
    mutex_unlock(&cn->mutex);
    return 0;
}



/* buffer io functions start */

int _conn_node_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt)
{
    int r;

    _conn_lock(cn);
    ICHK(LINFO, r, que_add(cn->n_rt_pkts, pkt));
    _conn_unlock(cn);

    L(LDEBUG, "+ _conn_node_tx_pkt %p(%d)\n", pkt, r);

    return r;
}

int _conn_router_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    _conn_lock(cn);

    //printf("!!! x\n");
    if(link_get_state(cn->link) == LINK_STATE_ALIVE){

        *pkt = que_get(cn->n_rt_pkts, &ts);

        if(*pkt){
            L(LINFO, "+ _conn_router_rx_pkt %p", *pkt);
            r = 0;
        }
    }

    _conn_unlock(cn);

    return r;
}

int _conn_router_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt)
{
    int r = 1;

    assert(pkt);

    _conn_lock(cn);
    CONN_GRP_PRE(cn);

    //printf("!!! a\n");;
    if(pkt_get_dest_grp_id(pkt) == grp->grp_id){
    //printf("!!! b\n");;
        if(link_get_state(cn->link) == LINK_STATE_ALIVE &&
                _conn_dec_rx_cnt(cn, grp, 1) != -1){

            ICHK(LINFO, r, que_add(cn->rt_n_pkts, pkt));
            done = 1;
        }
    }

    CONN_GRP_POST(cn);
    _conn_unlock(cn);


    L(LDEBUG, "+ _conn_router_tx_pkt %p(%d)\n", pkt, r);

    return r;
}


int _conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    *pkt = NULL;

    _conn_lock(cn);

    if(link_get_state(cn->link) == LINK_STATE_ALIVE){

        CONN_GRP_PRE(cn);

        *pkt = que_get(cn->rt_n_pkts, &ts);

        if(*pkt){
            r = 0;
            done = 1;
            // last read packet...
        }

        CONN_GRP_POST(cn);
    }

    _conn_unlock(cn);

    return r;
}


int _conn_set_rx_cnt(struct nn_conn *cn, int grp_id, int cnt)
{
    int r = 0;

    _conn_lock(cn);
    CONN_GRP_PRE(cn);

    if(grp->grp_id == grp_id){
        grp->rx_cnt = cnt;
        done = 1;
    }

    CONN_GRP_POST(cn);
    _conn_unlock(cn);

    return r;
}

static int _conn_dec_rx_cnt(struct nn_conn *cn, struct grps *grp, int dec)
{
    int r = -1;

    printf("!!! found\n");
    if(grp->rx_cnt != -2){
        grp->rx_cnt -= dec;
        if(grp->rx_cnt < -1){
            grp->rx_cnt = -1;
        }
    }
    r = grp->rx_cnt;

    printf("RRRR return %d\n", grp->rx_cnt);

    //CONN_GRP_POST(cn);
    //_conn_unlock(cn);

    return r;
}

static struct conn_grp_iter *conn_grp_iter_init(struct nn_conn *cn)
{
    return (struct conn_grp_iter *)ll_iter_init(cn->grps);
}

static int conn_grp_iter_free(struct conn_grp_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int conn_grp_iter_next(struct conn_grp_iter *iter, struct
        grps **grp)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)grp);
}

//    cn->rx_cnt = -2;
