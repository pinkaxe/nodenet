
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"
#include "node.h"
#include "link.h"
#include "grp.h"

#include "nn.h"


struct nn_router *nn_router_init(void)
{
    struct nn_router *rt;

    PCHK(LWARN, rt, router_init());

    return rt;
}


int nn_router_run(struct nn_router *rt)
{
    int r;

    router_lock(rt);

    ICHK(LWARN, r, router_run(rt));

    router_unlock(rt);

    return r;
}

struct nn_node *nn_node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    struct nn_node *n;

    PCHK(LWARN, n, node_init(type, attr, code, pdata));

    return n;
}

//int nn_node_setname(struct nn_node *n, char *name);

int nn_node_run(struct nn_node *n)
{
    int r;

    node_lock(n);

    ICHK(LWARN, r, node_start(n));

    node_unlock(n);

    return r;
}

struct nn_grp *nn_grp_init(int id)
{
    struct nn_grp *g;

    PCHK(LWARN, g, grp_init(id));

    return g;
}


int nn_join_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    grp_lock(g);
    node_lock(n);

    ICHK(LWARN, r, node_join_grp(n, g));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, grp_add_node(g, n));
    if(r){
        int rr;
        ICHK(LWARN, rr, node_quit_grp(n, g));
        goto err;
    }

err:
    //node_unlock(n);
    //grp_unlock(g);
    //rel_unlock();

    return r;
}

int nn_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    //rel_lock();
    //grp_lock(g);
    //node_lock(n);

    ICHK(LWARN, r, grp_rem_node(g, n));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, node_quit_grp(n, g));
    if(r){
        goto err;
    }

err:
    //node_unlock(n);
    //grp_unlock(g);
    //rel_unlock();

    return r;

}

int nn_router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb)
{
    int r;

    //router_lock(rt);

    ICHK(LWARN, r, router_set_cmd_cb(rt, cb));

    //router_unlock(rt);

    return r;
}

int nn_router_add_cmd_req(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r;

    //router_lock(rt);

    r = link_router_tx_cmd(rt, cmd);

    //router_unlock(rt);

    return r;
}

//int nn_router_set_data_cb(struct nn_router *rt, io_data_req_cb_t cb)
//{
//    int r;
//
//    ICHK(LWARN, r, router_set_data_cb(rt, cb));
//    return r;
//}

int nn_router_add_data_req(struct nn_router *rt, struct nn_io_data *data)
{
    int r;

    //r = router_add_data_req(rt, data);
    return r;
}



/* relationship between routers, nodes and grps */

/* when working with links between nodes and routers
 * - a connected router and node must never be locked simulatiously
 *   in the same thread(deadlock).
 * - to change a link, just change it on the side(router/node) that
 *   you have the link for. The link status etc. can be changed and
 *   can then be picked up by the other side when it accesses the
 *   link. */


static int _node_link_unlink(struct nn_node *n, struct nn_link *l)
{
    int r = 0;

    node_unlink(n, l);
    /* set to NULL for validation */
    link_set_node(l, NULL);

    return r;
}

static int _router_link_unlink(struct nn_router *rt, struct nn_link *l)
{
    int r = 0;

    router_unlink(rt, l);
    link_set_node(l, NULL);

    return r;
}

/* if the link was already disconnected, on return true, call
 * link_free(l) when locks unlocked */
static bool _link_mark_dead(struct nn_link *l)
{
    enum nn_link_state state;
    state = link_get_state(l);

    if(state == NN_LINK_STATE_DEAD){
        return true;
    }else{
        link_set_state(l, NN_LINK_STATE_DEAD);
        return false;
    }
}

int nn_link(struct nn_node *n, struct nn_router *rt)
{
    int r;
    struct nn_link *l;

    l = link_init();

    /* lock router and link */
    router_lock(rt);
    link_lock(l);

    /* set router side pointers */
    router_link(rt, l);
    link_set_router(l, rt);

    /* unlock router and link */
    link_unlock(l);
    router_unlock(rt);

    /* lock node and link */
    node_lock(n);
    link_lock(l);

    /* set node side pointers */
    node_link(n, l);
    link_set_node(l, n);

    /* unlock node and link */
    node_unlock(n);
    link_unlock(l);

err:

    return r;
}

int nn_unlink(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r;
    struct nn_link *l;
    bool f;

    /* lock node and each link it point to in turn */
    node_lock(n);

    iter = NULL;
    while((l=node_link_iter(n, &iter))){
        link_lock(l);

        /* disconnect the node <-> link link */
        _node_link_unlink(n, l);

        f = _link_mark_dead(l);

        /* unlock node and link */
        link_unlock(l);
        node_unlock(n);

        if(f){
            link_free(l);
            continue;
        }

        /* lock router and link */
        router_lock(rt);
        link_lock(l);

        /* disconnect the router <-> link link */
        _router_link_unlink(rt, l);

        f = _link_mark_dead(l);

        /* unlock router and link */
        link_unlock(l);
        router_unlock(rt);

        if(f){
            link_free(l);
            continue;
        }

        node_lock(n);
    }

err:
    node_unlock(n);

    return r;
}


int nn_grp_free(struct nn_grp *g)
{
    void *iter;
    int r;
    struct nn_node *node;

    // lock grp

    //while((n=grp_nodes_iter(g, &iter))){
    //    // unlock grp
    //    // lock n
    //    // lock grp
    //    // make sure still pointed at, grp_ismemb(g, n);
    //    ICHK(LWARN, r, node_quit_grp(n, g));
    //    // unlock n
    //    /* remove pointers from nodes to this grp */
    //}

    //unlock_grp();

    //rel_lock();

    ICHK(LWARN, r, grp_free(g));

    return r;
}

int nn_node_free(struct nn_node *n)
{
    void *iter = NULL;
    int r;
    struct nn_link *l;
    struct nn_router *rt;
    bool f = false; // free or not

    node_lock(n);

    /* remove the links to routers */
    while((l=node_link_iter(n, &iter))){

        link_lock(l);

        _node_link_unlink(n, l);

        f = _link_mark_dead(l);

        link_unlock(l);
        node_unlock(n);

        if(f){
            link_free(l);
        }

        node_lock(n);
    }

   // /* remove pointers from groups */
   // iter = NULL;
   // while((g=node_grps_iter(n, &iter))){
   //     grp_rem_node(g, n);
   // }

    node_unlock(n);

    ICHK(LWARN, r, node_free(n));

    return r;
}


int nn_router_free(struct nn_router *rt)
{
    void *iter = NULL;
    int r;
    struct nn_node *n;
    struct nn_link *l;
    bool f = false; // free or not

    router_lock(rt);

    while((l=router_link_iter(rt, &iter))){

        link_lock(l);

        _router_link_unlink(rt, l);

        f = _link_mark_dead(l);

        link_unlock(l);
        router_unlock(rt);

        if(f){
            link_free(l);
        }

        router_lock(rt);
    }

    router_unlock(rt);

    ICHK(LWARN, r, router_free(rt));

    return r;
}
