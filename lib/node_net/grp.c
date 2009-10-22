
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
#include "grp.h"


struct nn_grp_node {
    struct nn_node *node;
};

struct nn_grp {
    struct ll *node;
    int id;
    int node_no;
    mutex_t mutex;
};

int grp_isok(struct nn_grp *g)
{
    assert(g->node_no >= 0);
    assert(g->node >= 0);
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

    PCHK(LWARN, g->node, ll_init());
    if(!g->node){
        grp_free(g);
        goto err;
    }

    mutex_init(&g->mutex, NULL);

err:

    return g;
}

int grp_free(struct nn_grp *g)
{
    struct ll_iter *iter;
    int r;
    struct nn_grp_node *m;

    grp_isok(g);

    mutex_lock(&g->mutex);

    iter = ll_iter_init(g->node);
    if(g){
        if(g->node){
            // rem and free first
            while(!ll_iter_next(iter, (void **)&m)){
                ICHK(LWARN, r, ll_rem(g->node, m));
                free(m);
            }
            ICHK(LWARN, r, ll_free(g->node));
        }
        ll_iter_free(iter);

        mutex_unlock(&g->mutex);
        mutex_destroy(&g->mutex);
        free(g);
    }

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

int grp_add_node(struct nn_grp *g, struct nn_node *n)
{
    int r = 1;

    //grp_print(g);
    grp_isok(g);

    ICHK(LWARN, r, ll_add_front(g->node, (void *)&n));

    //grp_print(g);
    grp_isok(g);

    return r;
}

int grp_get_node(struct nn_grp *g, void **node, int max)
{
    struct nn_grp_node *m;
    int i = 0;
    struct ll_iter *iter;

    grp_isok(g);

    iter = ll_iter_init(g->node);
    while(!ll_iter_next(iter, (void **)&m)){
        // randomize?
        node[i++] = m;
    }
    ll_iter_free(iter);

    return 0;
}

int grp_isnode(struct nn_grp *g, struct nn_node *node)
{
    int r = 1;
    struct nn_grp_node *m;
    struct ll_iter *iter;

    grp_isok(g);

    iter = ll_iter_init(g->node);
    while(!ll_iter_next(iter, (void **)&m)){
        if(m->node == node){
            r = 0;
            break;
        }
    }
    ll_iter_free(iter);

    return r;
}

int grp_rem_node(struct nn_grp *g, struct nn_node *n)
{
    int r = 1;
    struct nn_node *_n;
    struct ll_iter *iter;

    grp_isok(g);

    iter = ll_iter_init(g->node);
    while(!ll_iter_next(iter, (void **)&_n)){
        if(_n == n){
            ICHK(LWARN, r, ll_rem(g->node, _n));
            grp_isok(g);
            grp_print(g);
            break;
        }
    }


    return r;
}

struct grp_node_iter *grp_node_iter_init(struct nn_grp *g)
{
    return (struct grp_node_iter *)ll_iter_init(g->node);
}

int grp_node_iter_free(struct grp_node_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

int grp_node_iter_next(struct grp_node_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}
