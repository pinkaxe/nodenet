
/* main api from outside */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"

#include "types.h"
#include "pkt.h"
#include "router.h"
#include "node.h"
#include "conn.h"
#include "grp.h"

#include "nn.h"


/* router */

struct nn_router *nn_router_init(void)
{
    struct nn_router *rt;

    PCHK(LWARN, rt, router_init());

    return rt;
}

int nn_router_free(struct nn_router *rt)
{
    int r;

    r = router_set_state(rt, NN_STATE_SHUTDOWN);

    return r;
}

int nn_router_clean(struct nn_router *rt)
{
    return router_clean(rt);
}

int nn_router_set_state(struct nn_router *rt, enum nn_state state)
{
    int r;

    ICHK(LWARN, r, router_set_state(rt, state));

    return r;
}

int nn_router_print(struct nn_router *rt)
{
    router_print(rt);
    return 0;
}


/* node */

struct nn_node *nn_node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    struct nn_node *n;

    PCHK(LWARN, n, node_init(type, attr, code, pdata));

    return n;
}

int nn_node_free(struct nn_node *n)
{
    return  node_set_state(n, NN_STATE_SHUTDOWN);
}

/* wait for node to be totally cleaned up */
int nn_node_clean(struct nn_node *n)
{
    return node_clean(n);
}

//int nn_node_setname(struct nn_node *n, char *name);

int nn_node_set_state(struct nn_node *n, enum nn_state state)
{
    int r;

    ICHK(LWARN, r, node_set_state(n, state));

    return r;
}

int nn_node_print(struct nn_node *n)
{
    node_print(n);
    return 0;
}


/* grp */

struct nn_grp *nn_grp_init(int id)
{
    struct nn_grp *g;

    PCHK(LWARN, g, grp_init(id));

    return g;
}

int nn_grp_free(struct nn_grp *g)
{
    int r;

    ICHK(LWARN, r, grp_free(g));

    return r;
}

int nn_join_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    grp_lock(g);
    //node_lock(n);

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
    grp_unlock(g);

    return r;
}

int nn_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    grp_lock(g);
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
    grp_unlock(g);

    return r;

}

/* NOTE: tx/rx shouldn't lock anything for now */

/*
int nn_node_tx_pkt(struct nn_node *n, struct nn_router *rt, struct nn_pkt
        *pkt)
{
    return conn_node_tx_pkt(n, rt, pkt);
}
*/

/*
int nn_router_tx_pkt(struct nn_router *rt, struct nn_node *n, struct nn_pkt *pkt)
{
    int r = 0;

    r = conn_router_tx_pkt(rt, n, pkt);

    return r;
}
*/


int nn_router_tx_data(struct nn_router *rt, struct nn_io_data *data)
{
    int r = 0;

    //r = router_add_data_req(rt, data);
    return r;
}


int nn_router_set_pkt_cb(struct nn_router *rt, io_pkt_req_cb_t cb)
{
    int r = 0;

    //router_lock(rt);

    //ICHK(LWARN, r, router_set_pkt_cb(rt, cb));

    //router_unlock(rt);

    return r;
}

int nn_node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt)
{
    return node_add_tx_pkt(n, pkt);
}

/*
int nn_node_get_tx_pkt(struct nn_node *n, struct nn_pkt **pkt)
{
    return node_get_tx_pkt(n, pkt);
}
*/

int nn_node_get_rx_pkt(struct nn_node *n, struct nn_pkt **pkt)
{
    return node_get_rx_pkt(n, pkt);
}

//int nn_router_set_data_cb(struct nn_router *rt, io_data_req_cb_t cb)
//{
//    int r;
//
//    ICHK(LWARN, r, router_set_data_cb(rt, cb));
//    return r;
//}


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


//#include "util/link.h"


int nn_conn(struct nn_node *n, struct nn_router *rt)
{
    struct nn_conn *cn;
    int r = 1;

    PCHK(LCRIT, cn, conn_init());
    if(!cn) goto err;

    ICHK(LCRIT, r, router_conn(rt, cn));
    if(r){
        free(cn);
        goto err;
    }

    ICHK(LCRIT, r, node_conn(n, cn));
    if(r){
        free(cn);
        goto err;
    }

err:
    return r;
}

int xnn_unconn(struct nn_node *n, struct nn_router *rt)
{
    int r = 0;
    struct nn_conn *cn;

    PCHK(LWARN, cn, node_get_router_conn(n, rt));

    if(cn){
        ICHK(LWARN, r, router_unconn(rt, cn));
        if(r){
            free(cn);
            goto err;
        }

        ICHK(LWARN, r, node_unconn(n, cn));
        if(r){
            free(cn);
            goto err;
        }
    }

err:
    conn_free(cn);

    return r;
}

