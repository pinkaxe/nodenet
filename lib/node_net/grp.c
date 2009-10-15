
#include <errno.h>
#include <assert.h>
#include <stdio.h>
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
}

int grp_print(struct nn_grp *g)
{
    struct nn_grp_node *gm;
    int r = 0;
    int c;
    void *iter;

    printf("\rt-- grp->node --: %p\rt\rt", g);
    c = 0;

    iter = NULL;
    while((gm=ll_next(g->node, &iter))){
        printf("p:%p\rt", gm->node);
        c++;
    }

    printf("total: %d\rt\rt", c);

    return 0;
}

struct nn_grp *grp_init(int id)
{
    int err;
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
    void *iter = NULL;
    int r;
    struct nn_grp_node *m;

    grp_isok(g);

    mutex_lock(&g->mutex);

    if(g){
        if(g->node){
            // rem and free first
            while(m=ll_next(g->node, &iter)){
                ICHK(LWARN, r, ll_rem(g->node, m));
                free(m);
            }
            ICHK(LWARN, r, ll_free(g->node));
        }
        free(g);
    }

    mutex_unlock(&g->mutex);
    mutex_destroy(&g->mutex);

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

int grp_add_node(struct nn_grp *h, struct nn_node *n)
{
    int r = 1;

    //grp_print(h);
    grp_isok(h);

    ICHK(LWARN, r, ll_add_front(h->node, (void *)&n));

    //grp_print(h);
    grp_isok(h);
err:
    return r;
}

int grp_get_node(struct nn_grp *h, void **node, int max)
{
    struct nn_grp_node *m;
    int i = 0;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while(m=ll_next(h->node, &iter)){
        // randomize?
        node[i++] = m;
    }

    return 0;
}

int grp_isnode(struct nn_grp *h, struct nn_node *node)
{
    int r = 1;
    struct nn_grp_node *m;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while(m=ll_next(h->node, &iter)){
        if(m->node == node){
            r = 0;
            break;
        }
    }

    return r;
}

int grp_rem_node(struct nn_grp *h, struct nn_node *n)
{
    int r = 1;
    struct nn_node *_n;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while((_n = ll_next(h->node, &iter))){
        if(_n == n){
            ICHK(LWARN, r, ll_rem(h->node, _n));
            grp_isok(h);
            grp_print(h);
            break;
        }
    }


    return r;
}

struct nn_node *grp_node_iter(struct nn_grp *g, void **iter)
{
    int r = 0;
    struct nn_node *n;
    struct nn_cmd *clone;

    assert(g);

    n = ll_next(g->node, iter);

    if(n){
        return n;
    }else{
        return NULL;
    }
}
