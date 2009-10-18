
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
#include "conn.h"
#include "grp.h"

#include "nn.h"

int busy_freeing_no = 0;

int nn_wait()
{
    while(busy_freeing_no > 0){
        usleep(500);
    }
}

struct nn_router *nn_router_init(void)
{
    struct nn_router *rt;

    PCHK(LWARN, rt, router_init());

    return rt;
}


int nn_router_set_state(struct nn_router *rt, enum nn_state state)
{
    int r;

    router_lock(rt);

    ICHK(LWARN, r, router_set_state(rt, state));
    router_cond_broadcast(rt);

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

int nn_node_set_state(struct nn_node *n, enum nn_state state)
{
    int r;

    node_lock(n);

    ICHK(LWARN, r, node_set_state(n, state));
    node_cond_broadcast(n);

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

int nn_node_tx_cmd(struct nn_node *n, struct nn_router *rt, struct nn_cmd
        *cmd)
{
    return conn_node_tx_cmd(n, rt, cmd);
}

int nn_router_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r;

    //router_lock(rt);

    //r = conn_router_tx_cmd(rt, cmd);

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

int nn_router_tx_data(struct nn_router *rt, struct nn_io_data *data)
{
    int r;

    //r = router_add_data_req(rt, data);
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


/* relationship between routers, nodes and grps, locking is intricate, work
 * with care */

/* when working with conns between nodes and routers
 * - a connected router and node must never be locked simulatiously
 *   in the same thread(deadlock).
 * - to change a conn, just change it on the side(router/node) that
 *   you have the conn for. The conn status etc. can be changed and
 *   can then be picked up by the other side when it accesses the
 *   conn.
 * - order of locking for node side - node, conn. For router
 *   side router, conn
 *   */


#include "util/link.h"


int nn_conn(struct nn_node *n, struct nn_router *rt)
{
    int r;
    struct nn_conn *cn;

    cn = conn_init();

    /* lock router and conn */
    router_lock(rt);
    conn_lock(cn);

    /* set router side pointers */
    router_conn(rt, cn);
    conn_set_router(cn, rt);

    /* unlock router and conn */
    conn_unlock(cn);
    router_unlock(rt);

    /* lock node and conn */
    node_lock(n);
    conn_lock(cn);

    /* set node side pointers */
    node_conn(n, cn);
    conn_set_node(cn, n);

    /* unlock node and conn */
    node_unlock(n);
    conn_unlock(cn);

err:

    return r;
}

int nn_unconn(struct nn_node *n, struct nn_router *rt)
{
    int r;
    bool f;

    NODE_CONN_ITER_PRE

    /* disconnect the node <-> conn conn */
    node_unconn(n, cn);
    conn_free_node(cn);

    NODE_CONN_ITER_POST

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
    struct nn_conn *cn;
    struct nn_router *rt;

    node_lock(n);

    busy_freeing_no++;
    r = node_set_state(n, NN_STATE_SHUTDOWN);
    node_cond_broadcast(n);

    node_unlock(n);

    return r;
}

int nn_node_clean(struct nn_node *n)
{

    node_lock(n);

    while(node_get_state(n) != NN_STATE_FINISHED){
        node_cond_wait(n);
    }

    node_unlock(n);

    node_free(n);

    return 0;
}


int nn_router_free(struct nn_router *rt)
{
    int r;

    router_lock(rt);

    busy_freeing_no++;
    r = router_set_state(rt, NN_STATE_SHUTDOWN);
    router_cond_broadcast(rt);

    router_unlock(rt);

    return r;
}

int nn_router_clean(struct nn_router *rt)
{

    router_lock(rt);

    while(router_get_state(rt) != NN_STATE_FINISHED){
        router_cond_wait(rt);
    }

    router_unlock(rt);

    router_free(rt);

    return 0;
}
