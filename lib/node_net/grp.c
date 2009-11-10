
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "sys/thread.h"

#include "util/ll.h"
#include "util/log.h"

#include "types.h"
#include "_grp_rel.h"
#include "grp.h"


struct nn_grp_grp_rel {
    struct nn_grp_rel *grp_rel;
};

struct nn_grp {
    struct ll *grp_rel;
    int id;
    int grp_rel_no;
    mutex_t mutex;
};

static struct grp_grp_rel_iter *grp_grp_rel_iter_init(struct nn_grp *g);
static int grp_grp_rel_iter_free(struct grp_grp_rel_iter *iter);
static int grp_grp_rel_iter_next(struct grp_grp_rel_iter *iter, struct
        nn_grp_rel **grp_rel);


#define GRP_GRP_REL_ITER_PRE \
    { \
    assert(g); \
    int done = 0; \
    struct grp_grp_rel_iter *iter; \
    struct nn_grp_rel *grp_rel; \
    grp_lock(g); \
    iter = grp_grp_rel_iter_init(g); \
    while(!done && !grp_grp_rel_iter_next(iter, &grp_rel)){ \
        _grp_rel_lock(grp_rel);

#define GRP_GRP_REL_ITER_POST \
        _grp_rel_unlock(grp_rel); \
    } \
    grp_grp_rel_iter_free(iter); \
    grp_unlock(g); \
    }

int grp_isok(struct nn_grp *g)
{
    assert(g->grp_rel_no >= 0);
    assert(g->grp_rel >= 0);
    return 0;
}

int grp_print(struct nn_grp *g)
{

    return 0;
}

struct nn_grp *grp_init(int id)
{
    struct nn_grp *g;

    PCHK(LWARN, g, calloc(1, sizeof(*g)));

    if(!g){
        goto err;
    }

    PCHK(LWARN, g->grp_rel, ll_init());
    if(!g->grp_rel){
        grp_free(g);
        goto err;
    }

    mutex_init(&g->mutex, NULL);

err:

    return g;
}

int grp_free(struct nn_grp *g)
{
    int r;

    grp_isok(g);

#if 0
    GRP_GRP_REL_ITER_PRE

    ICHK(LWARN, r, ll_rem(g->grp_rel, _grp_rel));

    GRP_GRP_REL_ITER_POST
#endif

    if(g->grp_rel){
        GRP_GRP_REL_ITER_PRE

        ICHK(LWARN, r, ll_rem(g->grp_rel, grp_rel));

        ICHK(LWARN, r, _grp_rel_free_grp(grp_rel));

        grp_isok(g);
        grp_print(g);

        GRP_GRP_REL_ITER_POST

        ll_free(g->grp_rel);
    }

    mutex_destroy(&g->mutex);
    free(g);

        /*
    mutex_lock(&g->mutex);

    iter = ll_iter_init(g->grp_rel);
    if(g){
        if(g->grp_rel){
            // rem and free first
            while(!ll_iter_next(iter, (void **)&m)){
                ICHK(LWARN, r, ll_rem(g->grp_rel, m));
                free(m);
            }
            ICHK(LWARN, r, ll_free(g->grp_rel));
        }
        ll_iter_free(iter);

        mutex_unlock(&g->mutex);
        mutex_destroy(&g->mutex);
        free(g);
    }
    */

    return 0;
}


int grp_lock(struct nn_grp *g)
{
    mutex_lock(&g->mutex);
    return 0;
}

int grp_unlock(struct nn_grp *g)
{
    mutex_unlock(&g->mutex);
    return 0;
}

int grp_add_grp_rel(struct nn_grp *g, struct nn_grp_rel *grp_rel)
{
    int r = 1;

    grp_isok(g);

    grp_lock(g);
    _grp_rel_lock(grp_rel);

    ICHK(LWARN, r, ll_add_front(g->grp_rel, (void **)&grp_rel));
    if(r) goto err;

    ICHK(LWARN, r, _grp_rel_set_grp(grp_rel, g));
    if(r) goto err;

err:
    _grp_rel_unlock(grp_rel);
    grp_unlock(g);

    return r;
}

int grp_rem_grp_rel(struct nn_grp *g, struct nn_grp_rel *_grp_rel)
{
    int r = 1;

    grp_isok(g);

    GRP_GRP_REL_ITER_PRE

    if(_grp_rel == grp_rel){

        ICHK(LWARN, r, ll_rem(g->grp_rel, grp_rel));

        ICHK(LWARN, r, _grp_rel_free_grp(grp_rel));

        grp_isok(g);
        grp_print(g);
        done = 1;
    }

    GRP_GRP_REL_ITER_POST


    return r;
}


int grp_get_grp_rel(struct nn_grp *g, void **grp_rel, int max)
{
    struct nn_grp_grp_rel *m;
    int i = 0;
    struct ll_iter *iter;

    grp_isok(g);

    iter = ll_iter_init(g->grp_rel);
    while(!ll_iter_next(iter, (void **)&m)){
        // randomize?
        grp_rel[i++] = m;
    }
    ll_iter_free(iter);

    return 0;
}

int grp_isgrp_rel(struct nn_grp *g, struct nn_grp_rel *grp_rel)
{
    int r = 1;
    struct nn_grp_grp_rel *m;
    struct ll_iter *iter;

    grp_isok(g);

    iter = ll_iter_init(g->grp_rel);
    while(!ll_iter_next(iter, (void **)&m)){
        if(m->grp_rel == grp_rel){
            r = 0;
            break;
        }
    }
    ll_iter_free(iter);

    return r;
}

static struct grp_grp_rel_iter *grp_grp_rel_iter_init(struct nn_grp *g)
{
    return (struct grp_grp_rel_iter *)ll_iter_init(g->grp_rel);
}

static int grp_grp_rel_iter_free(struct grp_grp_rel_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

static int grp_grp_rel_iter_next(struct grp_grp_rel_iter *iter, struct
        nn_grp_rel **grp_rel)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)grp_rel);
}
