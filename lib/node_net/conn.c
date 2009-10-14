
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"

#include "types.h"
#include "conn.h"
#include "cmd.h"
#include "router.h"

//struct nn_conn {
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

static int conn_free(struct nn_conn *cn);

struct nn_conn *conn_init()
{
    int r = 0;
    struct nn_conn *cn;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }

    mutex_init(&cn->mutex, NULL);

    PCHK(LWARN, cn->rt_n_cmd, que_init(8));
    if(!cn->rt_n_cmd){
        PCHK(LWARN, r, conn_free(cn));
        goto err;
    }

    PCHK(LWARN, cn->n_rt_cmd, que_init(8));
    if(!cn->n_rt_cmd){
        PCHK(LWARN, r, conn_free(cn));
        goto err;
    }

err:
    return cn;
}

int conn_set_node(struct nn_conn *cn, struct nn_node *n)
{
    cn->n = n;
    return 0;
}

int conn_set_router(struct nn_conn *cn, struct nn_router *rt)
{
    cn->rt = rt;
    return 0;
}

//    conn_set_router(cn, rt){
//        cn->rt = rt;
//    }
//
static int conn_free(struct nn_conn *cn)
{
    int r = 0;
    int fail = 0;

    if(cn->rt_n_cmd){
        ICHK(LWARN, r, que_free(cn->rt_n_cmd));
        if(r) fail++;
    }

    if(cn->n_rt_cmd){
        ICHK(LWARN, r, que_free(cn->n_rt_cmd));
        if(r) fail++;
    }

    free(cn);

    return fail;
}

int conn_lock(struct nn_conn *cn)
{
    mutex_lock(&cn->mutex);
}

int conn_unlock(struct nn_conn *cn)
{
    mutex_unlock(&cn->mutex);
}


int conn_create_node_router(struct nn_node *n, struct nn_router *rt)
{
    int r = 0;
    printf("!! create\n");
    return 0;

    // FIXME: put in nn.c
    struct nn_conn *cn;

    cn = conn_init();


    /* set router side */
    router_lock(rt);
    conn_lock(cn);

    cn->rt = rt;
    //router_add_conn(rt, cn);

    conn_unlock(cn);
    router_unlock(rt);

    /* set elem side */
    node_lock(n);
    conn_lock(cn);

    cn->n = n;
    //node_add_to_router(n, cn);

    node_unlock(n);
    conn_unlock(cn);

    conn_free(cn);

err:
    return r;
}

int conn_break_node_router(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r = 0;
    struct nn_conn *cn;

    printf("!! break\n");
    router_lock(rt);

    iter = NULL;
    while((cn=router_conn_iter(rt, &iter))){

        conn_lock(cn);
        cn->rt = NULL;
        //router_rem_conn(rt, cn);
        conn_unlock(cn);
        router_unlock(rt);

        node_lock(cn->n);
        conn_lock(cn);
        cn->n = NULL;

        //node_rem_from_router(cn->n, cn);
        conn_unlock(cn);
        node_unlock(cn->n);
    }

    //ICHK(LWARN, r, router_rem_memb(rt, n));
    //if(r){
    //    goto err;
    //}

    //node_lock(n);

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
            conn_free(cn);
            break;
        }else{
            conn_unlock(cn);
        }
    }
#endif

    //node_unlock(n);

err:
    return r;
}

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


