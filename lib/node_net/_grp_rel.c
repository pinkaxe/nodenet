
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"
#include "util/link.h"

#include "types.h"
#include "_grp_rel.h"
#include "pkt.h"

/* rename functions for this file */
/* FIXME: do for all *_from *_grp for this file */
#define link_free_node link_free_from
#define link_free_grp link_free_to

#define link_get_node link_get_from
#define link_get_grp link_get_to
#define link_set_node link_set_from
#define link_set_grp link_set_to

struct nn_grp_rel {
    struct link *link; /* keep link info, grp, n, state */
    mutex_t mutex;
    cond_t cond; /* if anything changes */
};


struct nn_grp_rel *_grp_rel_init()
{
    int r = 0;
    struct nn_grp_rel *grp_rel;
    struct link *l;

    PCHK(LWARN, grp_rel, calloc(1, sizeof(*grp_rel)));
    if(!grp_rel){
        goto err;
    }

    PCHK(LWARN, l, link_init());
    if(!l){
        PCHK(LWARN, r, _grp_rel_free(grp_rel));
        goto err;
    }
    grp_rel->link = l;

    mutex_init(&grp_rel->mutex, NULL);

    link_set_state(grp_rel->link, LINK_STATE_ALIVE);
err:
    return grp_rel;
}

int _grp_rel_free(struct nn_grp_rel *grp_rel)
{
    int fail = 0;

    /* IMPROVE: can save grp_rel state here */

    if(&grp_rel->mutex){
        mutex_destroy(&grp_rel->mutex);
    }

    free(grp_rel);

    return fail;
}

int _grp_rel_free_node(struct nn_grp_rel *grp_rel)
{
    int r;

    r = link_free_node(grp_rel->link);
    if(r == 1){ 
        grp_rel->link = NULL;
        _grp_rel_free(grp_rel);
    }

    return r;
}

int _grp_rel_free_grp(struct nn_grp_rel *grp_rel)
{
    int r;

    r = link_free_grp(grp_rel->link);
    if(r == 1){
        grp_rel->link = NULL;
        _grp_rel_free(grp_rel);
    }

    return r;
}


int _grp_rel_set_node(struct nn_grp_rel *grp_rel, struct nn_node *n)
{
    return link_set_from(grp_rel->link, n);
}

int _grp_rel_set_grp(struct nn_grp_rel *grp_rel, struct nn_grp *rt)
{
    return link_set_grp(grp_rel->link, rt);
}

int _grp_rel_set_state(struct nn_grp_rel *grp_rel, int state)
{
    return link_set_state(grp_rel->link, state);
}

struct nn_node *_grp_rel_get_node(struct nn_grp_rel *grp_rel)
{
    return link_get_node(grp_rel->link);
}

struct nn_grp *_grp_rel_get_grp(struct nn_grp_rel *grp_rel)
{
    return link_get_grp(grp_rel->link);
}

int _grp_rel_get_state(struct nn_grp_rel *grp_rel)
{
    return link_get_state(grp_rel->link);
}


int _grp_rel_lock(struct nn_grp_rel *grp_rel)
{
    mutex_lock(&grp_rel->mutex);
    return 0;
}

int _grp_rel_unlock(struct nn_grp_rel *grp_rel)
{
    mutex_unlock(&grp_rel->mutex);
    return 0;
}

