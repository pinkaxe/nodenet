
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

    ICHK(LCRIT, r, node_conn(n, cn));
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

    //PCHK(LWARN, g, router_get_grp(rt, grp_id));
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

        _conn_free(cn);
    }

err:

    return r;
}

int conn_join_grp(struct nn_conn *cn, int grp_id)
{
    int r;

    ICHK(LCRIT, r, _conn_join_grp(cn, grp_id));
    if(r){
    }

    router_add_to_grp(_conn_get_router(cn), grp_id, cn);
    //router_add_grp_memb(_conn_get_router(cn), grp_id, b);
}

int conn_quit_grp(struct nn_conn *cn, int grp_id)
{
    return _conn_quit_grp(cn, grp_id);
}

int xconn_conn(struct nn_node *n, struct nn_router *rt)
{
    struct nn_conn *cn;
    int r = 1;

    PCHK(LCRIT, cn, _conn_init());
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

int xconn_unconn(struct nn_node *n, struct nn_router *rt)
{
    int r = 1;
    struct nn_grp *g;
    struct nn_conn *cn;

#if 0
    //PCHK(LWARN, g, router_get_grp(rt, grp_id));
    PCHK(LWARN, cn, node_get_router_conn(n, g));

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

        _conn_free(cn);
    }
#endif

err:

    return r;
}
