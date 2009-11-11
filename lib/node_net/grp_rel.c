
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"

#include "types.h"
#include "pkt.h" /* FIXME: */
#include "node.h"
#include "_grp_rel.h"
#include "grp.h"

int grp_join(struct nn_node *n, struct nn_grp *grp)
{
    struct nn_grp_rel *grp_rel;
    int r = 1;

    PCHK(LCRIT, grp_rel, _grp_rel_init());
    if(!grp_rel) goto err;

    ICHK(LCRIT, r, grp_add_grp_rel(grp, grp_rel));
    if(r){
        free(grp_rel);
        goto err;
    }

    ICHK(LCRIT, r, node_join_grp(n, grp_rel));
    if(r){
        free(grp_rel);
        goto err;
    }

err:
    return r;
}

int grp_quit(struct nn_node *n, struct nn_grp *grp)
{
    int r = 0;
    struct nn_grp_rel *grp_rel;

    PCHK(LWARN, grp_rel, node_get_grp_rel(n, grp));

    if(grp_rel){
        ICHK(LWARN, r, grp_rem_grp_rel(grp, grp_rel));
        if(r){
            goto err;
        }

        ICHK(LWARN, r, node_quit_grp(n, grp_rel));
        if(r){
            goto err;
        }
    }

err:
    _grp_rel_free(grp_rel);

    return r;
}
