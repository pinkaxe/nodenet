
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"

#include "types.h"
#include "router.h"
#include "node.h"
#include "_conn.h"
#include "conn.h"

int conn_conn(struct nn_node *n, struct nn_router *rt)
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

int conn_unconn(struct nn_node *n, struct nn_router *rt)
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
    _conn_free(cn);

    return r;
}
