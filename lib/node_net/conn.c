
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"
#include "util/link.h"

#include "types.h"
#include "node.h"
#include "conn.h"
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
    struct link *link;
    mutex_t mutex;
    cond_t cond; /* if anything changes */

    /* router(output) -> node(input) */
    struct que *rt_n_pkts;   /* router write node pkt */

    /* node(output) -> router(input) */
    struct que *n_rt_pkts;   /* n write data */
    struct que *n_rt_notify; /* always used, from node_driver */
    //struct que *n_rt_pkts;    /* only master node */

};


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
            //PCHK(LWARN, r, conn_free(cn));
            //goto err;
            // FIXME: free?
            r = 1;
            break;
        }
        _qs++;
    }

    return r;
}

struct nn_conn *conn_init()
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
        conn_free(cn);
        cn = NULL;
        goto err;
    }
    assert(cn->rt_n_pkts);
    assert(cn->n_rt_pkts);

    PCHK(LWARN, l, link_init());
    if(!l){
        PCHK(LWARN, r, conn_free(cn));
        goto err;
    }
    cn->link = l;

    mutex_init(&cn->mutex, NULL);

    link_set_state(cn->link, LINK_STATE_ALIVE);
err:
    return cn;
}

int conn_free(struct nn_conn *cn)
{
    int r = 0;
    int fail = 0;
    struct timespec ts = {0, 0};
    struct nn_pkt *pkt;

    /* IMPROVE: can save conn state here */

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

    if(&cn->mutex){
        mutex_destroy(&cn->mutex);
    }

    free(cn);

    return fail;
}

int conn_free_node(struct nn_conn *cn)
{
    int r;

    r = link_free_node(cn->link);
    return r;
}

int conn_free_router(struct nn_conn *cn)
{
    int r;

    r = link_free_router(cn->link);

    return r;
}


int conn_set_node(struct nn_conn *cn, struct nn_node *n)
{
    return link_set_from(cn->link, n);
}

int conn_set_router(struct nn_conn *cn, struct nn_router *rt)
{
    return link_set_router(cn->link, rt);
}

int conn_set_state(struct nn_conn *cn, int state)
{
    return link_set_state(cn->link, state);
}

struct nn_node *conn_get_node(struct nn_conn *cn)
{
    return link_get_node(cn->link);
}

struct nn_router *conn_get_router(struct nn_conn *cn)
{
    return link_get_router(cn->link);
}

int conn_get_state(struct nn_conn *cn)
{
    return link_get_state(cn->link);
}


int conn_lock(struct nn_conn *cn)
{
    mutex_lock(&cn->mutex);
    return 0;
}

int conn_unlock(struct nn_conn *cn)
{
    mutex_unlock(&cn->mutex);
    return 0;
}


/* buffer io functions start */



int conn_node_tx_pkt(struct nn_node *n, struct nn_router *rt, struct nn_pkt
        *pkt)
{
    int r = 0;

    NODE_CONN_ITER_PRE

    if(link_get_router(cn->link) == rt){

        r = que_add(cn->n_rt_pkts, pkt);

        // FIXME: signal router of change in some way
    }

    NODE_CONN_ITER_POST

    return r;
}

int conn_router_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    if(link_get_state(cn->link) == LINK_STATE_ALIVE){
        *pkt = que_get(cn->n_rt_pkts, &ts);
        if(*pkt) r = 0;
    }

    return r;
}

/* router -> node pkt */
int conn_router_tx_pkt(struct nn_router *rt, struct nn_node *n, struct nn_pkt
        *pkt)
{

    int r = 1;

    NODE_CONN_ITER_PRE

    if(link_get_router(cn->link) == rt){
        r = que_add(cn->rt_n_pkts, pkt);
        //node_set_pkt_avail
    }

    NODE_CONN_ITER_POST

    return r;

}


int conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt)
{
    int r = 1;
    struct timespec ts = {0, 0};

    *pkt = NULL;
    if(link_get_state(cn->link) == LINK_STATE_ALIVE){
        *pkt = que_get(cn->rt_n_pkts, &ts);
        if(*pkt){
            printf("pkt: %p\n", *pkt);
            r = 0;
        }
    }

    return r;
}
