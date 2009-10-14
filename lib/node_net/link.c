
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"

#include "types.h"
#include "link.h"
#include "cmd.h"
#include "router.h"

//struct nn_link {
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

static int link_free(struct nn_link *cn);

struct nn_link *link_init()
{
    int r = 0;
    struct nn_link *cn;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }

    mutex_init(&cn->mutex, NULL);

    PCHK(LWARN, cn->rt_n_cmd, que_init(8));
    if(!cn->rt_n_cmd){
        PCHK(LWARN, r, link_free(cn));
        goto err;
    }

    PCHK(LWARN, cn->n_rt_cmd, que_init(8));
    if(!cn->n_rt_cmd){
        PCHK(LWARN, r, link_free(cn));
        goto err;
    }

err:
    return cn;
}

int link_set_node(struct nn_link *cn, struct nn_node *n)
{
    cn->n = n;
    return 0;
}

int link_set_router(struct nn_link *cn, struct nn_router *rt)
{
    cn->rt = rt;
    return 0;
}

//    link_set_router(cn, rt){
//        cn->rt = rt;
//    }
//
static int link_free(struct nn_link *cn)
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

int link_lock(struct nn_link *cn)
{
    mutex_lock(&cn->mutex);
}

int link_unlock(struct nn_link *cn)
{
    mutex_unlock(&cn->mutex);
}


int link_create_node_router(struct nn_node *n, struct nn_router *rt)
{
    int r = 0;
    printf("!! create\n");
    return 0;

    // FIXME: put in nn.c
    struct nn_link *cn;

    cn = link_init();


    /* set router side */
    router_lock(rt);
    link_lock(cn);

    cn->rt = rt;
    //router_add_link(rt, cn);

    link_unlock(cn);
    router_unlock(rt);

    /* set elem side */
    node_lock(n);
    link_lock(cn);

    cn->n = n;
    //node_add_to_router(n, cn);

    node_unlock(n);
    link_unlock(cn);

    link_free(cn);

err:
    return r;
}

int link_break_node_router(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r = 0;
    struct nn_link *cn;

    printf("!! break\n");
    router_lock(rt);

    iter = NULL;
    while((cn=router_link_iter(rt, &iter))){

        link_lock(cn);
        cn->rt = NULL;
        //router_rem_link(rt, cn);
        link_unlock(cn);
        router_unlock(rt);

        node_lock(cn->n);
        link_lock(cn);
        cn->n = NULL;

        //node_rem_from_router(cn->n, cn);
        link_unlock(cn);
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
        link_lock(cn);
        if(cn->rt == rt){

            /* update with node and link locked */
            node_rem_router_link(cn->n);
            cn->n = NULL;

            link_unlock(cn);
            node_unlock(n);
            /* everything unlocked again */

            /* update router */
            router_lock(cn->rt)
            link_lock();

            /* update with router and link locked */
            router_rem_node_link(cn->rt);
            rt = cn->rt;
            cn->rt = NULL;

            link_unlock(cn);
            router_unlock(rt)
            /* everything unlocked again */

            /* free the link */
            link_free(cn);
            break;
        }else{
            link_unlock(cn);
        }
    }
#endif

    //node_unlock(n);

err:
    return r;
}

/* router -> node cmd */
int link_router_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    // add to rt_n_cmd
}

int link_node_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd)
{
    // add to rt_n_cmd
}

/* router -> node data */
int link_router_tx_data(struct nn_router *rt, struct nn_data *data)
{
    // add to rt_n_data
}

int link_node_rx_data(struct nn_router *rt, struct nn_data **data)
{
    // remove from rt_n_data
}

/* node -> router cmd */
int link_node_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    // add to n_rt_cmd
}

int link_router_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd)
{
    // remove from n_rt_cmd
}


/* node -> router data */
int link_node_tx_data(struct nn_router *rt, struct nn_data *data)
{
    // add to n_rt_data
}

int link_router_rx_data(struct nn_router *rt, struct nn_data **data)
{
    // remove from n_rt_data
}

/* router -> node notify */
int link_node_tx_notify(struct nn_router *rt, struct nn_notify *notify)
{
    // add to n_rt_notify
}

int link_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
{
    // remove from n_rt_notify
}


