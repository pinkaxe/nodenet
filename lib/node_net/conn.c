
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "conn.h"
#include "cmd.h"
#include "router.h"

//struct nn_conn_node_router {
//    struct nn_node *n;
//    struct nn_router *rt;
//    mutex_t mutex;
//
//    /* router(output) -> node(input) */
//    struct que *rt_n_cmd;   /* router write node cmd */
//    struct que *rt_n_data;  /* router write node data */
//
//    /* node(output) -> router(input) */
//    struct que *n_rt_notify; /* always used */
//    struct que *n_rt_cmd;    /* only master node */
//    struct que *n_rt_data;   /* n write data */
//
//    /* internal going to and from router commands */
//    struct que *n_rt_int_cmd;
//    struct que *rt_n_int_cmd;
//};

struct nn_data; // FIXME:

/* router -> node cmd */
int conn_router_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    // add to rt_n_cmd
}

int conn_node_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd)
{
    // add to rt_n_cmd
}

/* router -> node data */
int conn_router_tx_data(struct nn_router *rt, struct nn_data *data)
{
    // add to rt_n_data
}

int conn_node_rx_data(struct nn_router *rt, struct nn_data **data)
{
    // remove from rt_n_data
}

/* node -> router cmd */
int conn_node_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd)
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

/* router -> node notify */
int conn_node_tx_notify(struct nn_router *rt, struct nn_notify *notify)
{
    // add to n_rt_notify
}

int conn_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
{
    // remove from n_rt_notify
}


static struct nn_conn_node_router *conn_node_router_init()
{
    struct nn_conn_node_router *cn;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }

    mutex_init(&cn->mutex, NULL);

err:
    return cn;
}

#if 0
static int conn_node_router_set_node(struct nn_conn_node_router *cn)
{
    cn->n = n;
    return 0;
}

static int conn_node_router_set_router(struct nn_conn_node_router *cn)
{
    cn->rt = rt;
    return 0;
}
#endif


static int conn_node_router_free(struct nn_conn_node_router *cn)
{
    free(cn);
    return 0;
}

static int conn_node_router_lock(struct nn_conn_node_router *cn)
{
    mutex_lock(&cn->mutex);
}

static int conn_node_router_unlock(struct nn_conn_node_router *cn)
{
    mutex_unlock(&cn->mutex);
}


int conn_create_node_router(struct nn_node *n, struct nn_router *rt)
{
    int r = 0;

    struct nn_conn_node_router *cn;

    cn = conn_node_router_init();

    conn_node_router_lock(cn);

    /* set router side */
    router_lock(rt);

    cn->rt = rt;
    //router_add_memb(rt, cn);

    router_unlock(rt);

    /* set elem side */
    node_lock(n);

    cn->n = n;
    //node_add_to_router(n, cn);

    node_unlock(n);

    conn_node_router_unlock(cn);

    conn_node_router_free(cn);

err:
    return r;
}

int conn_break_node_router(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r;
    struct nn_conn_node_router *cn;

    //ICHK(LWARN, r, router_rem_memb(rt, n));
    //if(r){
    //    goto err;
    //}

    node_lock(n);

#if 0
    iter = NULL;
    while(cn=ll_next(n->routers, &iter)){
        conn_lock(cn);
        if(cn->rt == rt){

            /* update with node and conn locked */
            node_rem_router_conn(cn->n);
            cn->n = NULL;

            conn_unlock(cn);
            node_unlock(n);
            /* everything unlocked again */

            /* update router */
            router_lock(cn->rt)
            conn_lock();

            /* update with router and conn locked */
            router_rem_node_conn(cn->rt);
            rt = cn->rt;
            cn->rt = NULL;

            conn_unlock(cn);
            router_unlock(rt)
            /* everything unlocked again */

            /* free the conn */
            conn_node_router_free(cn);
            break;
        }else{
            conn_unlock(cn);
        }
    }
#endif

    node_unlock(n);

err:
    return r;
}
