
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"

#include "types.h"
#include "pkt.h"
#include "router.h"
#include "node.h"
#include "_conn.h"
#include "conn.h"

struct nn_conn *conn_conn(struct nn_node *n, struct nn_router *rt)
{
    struct nn_conn *cn;
    int r = 1;

    PCHK(LCRIT, cn, _conn_init());
    if(!cn) goto err;

    ICHK(LCRIT, r, router_conn(rt, cn));
    if(r){
        free(cn);
        cn = NULL;
        goto err;
    }

    r = _conn_set_router(cn, rt);
    if(r){
        free(cn);
        cn = NULL;
        goto err;
    }

    ICHK(LCRIT, r, node_conn(n, cn));
    if(r){
        free(cn);
        cn = NULL;
        goto err;
    }

    r = _conn_set_node(cn, n);
    if(r){
        free(cn);
        cn = NULL;
        goto err;
    }

err:
    return cn;
}

int conn_unconn(struct nn_node *n, struct nn_router *rt)
{
    int r = 1;
    struct nn_conn *cn;

    //PCHK(LWARN, g, router_get_chan(rt, chan_id));
    PCHK(LWARN, cn, node_get_router_conn(n, rt));

    if(cn){
        ICHK(LWARN, r, router_unconn(rt, cn));
        ICHK(LWARN, r, node_unconn(n, cn));

        ICHK(LWARN, r, _conn_free_router(cn));
        if(r){
            goto err;
        }

        ICHK(LWARN, r, _conn_free_node(cn));
        if(r){
            goto err;
        }

    }

err:

    return r;
}


/* joining a channel add a cn->conn and link with routers chan */
#if 0
int node_join_chan(struct nn_node *n, struct nn_conn *cn, int chan_id)
{
    int r;

    ICHK(LCRIT, r, _conn_join_chan(cn, chan_id));
    if(r){
    }

    router_add_to_chan(_conn_get_router(cn), chan_id, cn);

}
#endif

int conn_join_chan(struct nn_conn *cn, int chan_id)
{
    int r;

    ICHK(LCRIT, r, _conn_join_chan(cn, chan_id));
    if(r){
    }

    router_add_to_chan(_conn_get_router(cn), chan_id, cn);
    //router_add_chan_memb(_conn_get_router(cn), chan_id, b);
    return r;
}

int conn_quit_chan(struct nn_conn *cn, int chan_id)
{
    return _conn_quit_chan(cn, chan_id);
}
