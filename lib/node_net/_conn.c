
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"
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

struct nn_conn {
    struct link *link; /* keep link info, rt, n, state */
    mutex_t mutex;
    cond_t cond; /* if anything changes */

    /* router(output) -> node(input) */
    struct que *rt_n_pkts;   /* router write node pkt */

    /* node(output) -> router(input) */
    struct que *n_rt_pkts;   /* n write data */
    struct que *n_rt_notify; /* always used, from node_driver */
    //struct que *n_rt_pkts;    /* only master node */

    int grp_id;
    int rx_cnt; /* counter to only allow a certain amount of rx
                   buffers -2=unlimited(default) */

};

static int _conn_lock(struct nn_conn *cn);
static int _conn_unlock(struct nn_conn *cn);
static int _conn_dec_rx_cnt(struct nn_conn *cn, int dec);


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
        goto err;
    }
    cn->link = l;
    printf("xx %p\n", cn->link);

    cn->rx_cnt = -2;

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

int _conn_set_router(struct nn_conn *cn, struct nn_grp *g, int grp_id)
{
    int r;

    _conn_lock(cn);
    r = link_set_router(cn->link, g);
    cn->grp_id = grp_id;
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

struct nn_grp *_conn_get_router(struct nn_conn *cn)
{
    struct nn_grp *g = NULL;

    _conn_lock(cn);
    g = link_get_router(cn->link);
    _conn_unlock(cn);

    return g;
}

int _conn_get_grp_id(struct nn_conn *cn)
{
    int r;

    _conn_lock(cn);
    r = cn->grp_id;
    _conn_unlock(cn);

    return r;
}

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

    _conn_lock(cn);

    if(link_get_state(cn->link) == LINK_STATE_ALIVE &&
            _conn_dec_rx_cnt(cn, 1) != -1){

        ICHK(LINFO, r, que_add(cn->rt_n_pkts, pkt));
    }

    _conn_unlock(cn);

    L(LDEBUG, "+ _conn_router_tx_pkt %p(%d)\n", pkt, r);

    return r;
}


int _conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    *pkt = NULL;


    if(link_get_state(cn->link) == LINK_STATE_ALIVE){

        _conn_lock(cn);
        *pkt = que_get(cn->rt_n_pkts, &ts);
        _conn_unlock(cn);

        if(*pkt){
            r = 0;
        }
    }

    return r;
}


int _conn_set_rx_cnt(struct nn_conn *cn, int cnt)
{
    int r = 0;

    _conn_lock(cn);
    cn->rx_cnt = cnt;
    _conn_unlock(cn);

    return r;
}

static int _conn_dec_rx_cnt(struct nn_conn *cn, int dec)
{
    if(cn->rx_cnt != -2){
        cn->rx_cnt -= dec;
        if(cn->rx_cnt < -1){
            cn->rx_cnt = -1;
        }
    }
    printf("RRRR return %d\n", cn->rx_cnt);
    return cn->rx_cnt;
}
