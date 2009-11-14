
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

int conn_conn(struct nn_node *n, struct nn_router *rt, int grp_id)
{
    struct nn_conn *cn;
    int r = 1;

    PCHK(LCRIT, cn, _conn_init());
    if(!cn) goto err;

    ICHK(LCRIT, r, router_conn(rt, grp_id, cn));
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

int conn_unconn(struct nn_node *n, struct nn_router *rt, int grp_id)
{
    int r = 0;
    struct nn_grp *g;
    struct nn_conn *cn;

    PCHK(LWARN, g, router_get_grp(rt, grp_id));
    PCHK(LWARN, cn, node_get_router_conn(n, g));

    if(cn){
        ICHK(LWARN, r, router_unconn(rt, grp_id, cn));
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
