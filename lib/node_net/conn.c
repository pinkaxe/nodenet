
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"
#include "util/link.h"

#include "types.h"
#include "router.h"
#include "node.h"
#include "conn.h"
#include "cmd.h"


struct nn_conn {
    struct link *link;
    mutex_t mutex;

    /* router(output) -> node(input) */
    struct que *rt_n_cmd;   /* router write node cmd */
    struct que *rt_n_data;  /* router write node data */

    /* node(output) -> router(input) */
    struct que *n_rt_notify; /* always used */
    struct que *n_rt_cmd;    /* only master node */
    struct que *n_rt_data;   /* n write data */

    /* internal going to and from router commands */
    struct que *n_rt_icmd;
    struct que *rt_n_icmd;
};

struct nn_data; // FIXME:

static int conn_free(struct nn_conn *cn);

struct ques_to_init {
    struct que *q;
    size_t size;
};

int ques_init(struct ques_to_init *qs)
{
    int r = 0;
    struct ques_to_init *_qs = qs;

    while((_qs->q)){
        PCHK(LWARN, _qs->q, que_init(8));
        if(!_qs->q){
            //PCHK(LWARN, r, conn_free(cn));
            //goto err;
            // FIXME: free?
            r = 1;
            break;
        }
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

    struct ques_to_init qs[] = {
        {cn->rt_n_icmd, 8},
        {cn->n_rt_icmd, 8},
        {cn->rt_n_cmd, 8},
        {cn->n_rt_cmd, 8},
        {NULL, NULL},
    };

    r = ques_init(&qs);
    if(r){
        conn_free(cn);
        cn = NULL;
        goto err;
    }

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

static int conn_free(struct nn_conn *cn)
{
    int r = 0;
    int fail = 0;

    /* IMPROVE: can save conn state here */

    if(cn->rt_n_cmd){
        ICHK(LWARN, r, que_free(cn->rt_n_cmd));
        if(r) fail++;
    }

    if(cn->n_rt_cmd){
        ICHK(LWARN, r, que_free(cn->n_rt_cmd));
        if(r) fail++;
    }

    if(cn->link){
        free(cn->link);
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

    r = link_free_from(cn->link);
    if(r == 1){
        conn_free(cn);
    }

}

int conn_free_router(struct nn_conn *cn)
{
    int r;

    r = link_free_to(cn->link);
    if(r == 1){
        conn_free(cn);
    }

    return 0;
}


int conn_set_node(struct nn_conn *cn, struct nn_node *n)
{
    return link_set_from(cn->link, n);
}

int conn_set_router(struct nn_conn *cn, struct nn_router *rt)
{
    return link_set_to(cn->link, rt);
}

int conn_set_state(struct nn_conn *cn, int state)
{
    return link_set_state(cn->link, state);
}

struct nn_node *conn_get_node(struct nn_conn *cn)
{
    return link_get_from(cn->link);
}

struct nn_router *conn_get_router(struct nn_conn *cn)
{
    return link_get_to(cn->link);
}

int conn_get_state(struct nn_conn *cn)
{
    return link_get_state(cn->link);
}


int conn_lock(struct nn_conn *cn)
{
    mutex_lock(&cn->mutex);
}

int conn_unlock(struct nn_conn *cn)
{
    mutex_unlock(&cn->mutex);
}


/* buffer io functions start */

/* n != NULL when this is called, afterwards cn for the
 matching each matching router is set and can be used */
#define NODE_CONN_ITER_PRE \
    assert(n); \
    void *iter = NULL; \
    struct nn_conn *cn; \
    node_lock(n); \
    while((cn = node_conn_iter(n, &iter))){ \
        conn_lock(cn);

#define NODE_CONN_ITER_POST \
        conn_unlock(cn); \
    } \
    node_unlock(n);


/* rt != NULL when this is called, afterwards cn for the
 matching each matching node is set and can be used */
#define ROUTER_CONN_ITER_PRE \
    assert(rt); \
    void *iter = NULL; \
    struct nn_conn *cn; \
    router_lock(rt); \
    while((cn = router_conn_iter(n, &iter))){ \
        conn_lock(cn);

#define ROUTER_CONN_ITER_POST \
        conn_unlock(cn); \
    } \
    router_unlock(n);


/* router -> node cmd */
int conn_router_tx_icmd(struct nn_router *rt, struct nn_node *n, struct nn_icmd
        *icmd)
{

    int r = 1;

    NODE_CONN_ITER_PRE

    if(link_get_to(cn->link) == rt){
        que_add(cn->rt_n_icmd, icmd);
    }

    NODE_CONN_ITER_POST

    return r;

}

int conn_node_rx_icmd(struct nn_node *n, struct nn_router *rt, struct nn_icmd
        **icmd)
{

    NODE_CONN_ITER_PRE

    if(link_get_to(cn->link) == rt){
        //que_get(cn->rt_n_icmd, icmd);
    }

    NODE_CONN_ITER_POST

}

#if 0
/* router -> node data */
int conn_router_tx_data(struct nn_router *rt, struct nn_data *data)
{
    // add to rt_n_data
}

int conn_node_rx_data(struct nn_node *rt, struct nn_data **data)
{
    // remove from rt_n_data
}

/* node -> router cmd */
int conn_node_tx_cmd(struct nn_node *rt, struct nn_cmd *cmd)
{
    // add to n_rt_cmd
}

int conn_router_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd)
{
    // remove from n_rt_cmd
}


/* node -> router data */
int conn_node_tx_data(struct nn_router *rt, struct nn_data *data)
{
    // add to n_rt_data
}

int conn_router_rx_data(struct nn_router *rt, struct nn_data **data)
{
    // remove from n_rt_data
}

#if 0
/* router -> node notify */
int conn_node_tx_notify(struct nn_router *rt, struct nn_notify *notify)
{
    // add to n_rt_notify
}

int conn_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
{
    // remove from n_rt_notify
}

#endif
#endif
