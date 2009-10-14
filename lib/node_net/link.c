
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

static int link_free(struct nn_link *l);

struct nn_link *link_init()
{
    int r = 0;
    struct nn_link *l;

    PCHK(LWARN, l, calloc(1, sizeof(*l)));
    if(!l){
        goto err;
    }

    mutex_init(&l->mutex, NULL);

    PCHK(LWARN, l->rt_n_cmd, que_init(8));
    if(!l->rt_n_cmd){
        PCHK(LWARN, r, link_free(l));
        goto err;
    }

    PCHK(LWARN, l->n_rt_cmd, que_init(8));
    if(!l->n_rt_cmd){
        PCHK(LWARN, r, link_free(l));
        goto err;
    }

err:
    return l;
}

int link_set_node(struct nn_link *l, struct nn_node *n)
{
    l->n = n;
    return 0;
}

int link_set_router(struct nn_link *l, struct nn_router *rt)
{
    l->rt = rt;
    return 0;
}

//    link_set_router(l, rt){
//        l->rt = rt;
//    }
//
static int link_free(struct nn_link *l)
{
    int r = 0;
    int fail = 0;

    if(l->rt_n_cmd){
        ICHK(LWARN, r, que_free(l->rt_n_cmd));
        if(r) fail++;
    }

    if(l->n_rt_cmd){
        ICHK(LWARN, r, que_free(l->n_rt_cmd));
        if(r) fail++;
    }

    free(l);

    return fail;
}

int link_lock(struct nn_link *l)
{
    mutex_lock(&l->mutex);
}

int link_unlock(struct nn_link *l)
{
    mutex_unlock(&l->mutex);
}


int link_create_node_router(struct nn_node *n, struct nn_router *rt)
{
    int r = 0;
    printf("!! create\n");
    return 0;

    // FIXME: put in nn.c
    struct nn_link *l;

    l = link_init();


    /* set router side */
    router_lock(rt);
    link_lock(l);

    l->rt = rt;
    //router_conn(rt, l);

    link_unlock(l);
    router_unlock(rt);

    /* set elem side */
    node_lock(n);
    link_lock(l);

    l->n = n;
    //node_add_to_router(n, l);

    node_unlock(n);
    link_unlock(l);

    link_free(l);

err:
    return r;
}

int link_break_node_router(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r = 0;
    struct nn_link *l;

    printf("!! break\n");
    router_lock(rt);

    iter = NULL;
    while((l=router_conn(rt, &iter))){

        link_lock(l);
        l->rt = NULL;
        //router_dconn(rt, l);
        link_unlock(l);
        router_unlock(rt);

        node_lock(l->n);
        link_lock(l);
        l->n = NULL;

        //node_rem_from_router(l->n, l);
        link_unlock(l);
        node_unlock(l->n);
    }

    //ICHK(LWARN, r, router_rem_memb(rt, n));
    //if(r){
    //    goto err;
    //}

    //node_lock(n);

#if 0
    iter = NULL;
    while(l=ll_next(n->routers, &iter)){
        link_lock(l);
        if(l->rt == rt){

            /* update with node and link locked */
            node_rem_router_link(l->n);
            l->n = NULL;

            link_unlock(l);
            node_unlock(n);
            /* everything unlocked again */

            /* update router */
            router_lock(l->rt)
            link_lock();

            /* update with router and link locked */
            router_rem_node_link(l->rt);
            rt = l->rt;
            l->rt = NULL;

            link_unlock(l);
            router_unlock(rt)
            /* everything unlocked again */

            /* free the link */
            link_free(l);
            break;
        }else{
            link_unlock(l);
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


