
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "util/ll.h"
#include "util/log.h"

#include "types.h"
#include "grp.h"


struct nn_grp_memb {
    struct nn_node *memb;
};

struct nn_grp {
    struct ll *memb;
    int id;
    int memb_no;
};

int grp_isok(struct nn_grp *g)
{
    assert(g->memb_no >= 0);
    assert(g->memb >= 0);
}

int grp_print(struct nn_grp *g)
{
    struct nn_grp_memb *gm;
    int r = 0;
    int c;
    void *iter;

    printf("\rt-- grp->node --: %p\rt\rt", g);
    c = 0;

    iter = NULL;
    while((gm=ll_next(g->memb, &iter))){
        printf("p:%p\rt", gm->memb);
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

    PCHK(LWARN, g->memb, ll_init());
    if(!g->memb){
        grp_free(g);
        goto err;
    }

err:

    return g;
}

int grp_free(struct nn_grp *g)
{
    int r;

    grp_isok(g);

    if(g){
        if(g->memb){
            ICHK(LWARN, r, ll_free(g->memb));
        }
        free(g);
    }


    return 0;
}


int grp_add_memb(struct nn_grp *h, struct nn_node *memb)
{
    int r = 1;
    struct nn_grp_memb *m;

    //grp_print(h);
    grp_isok(h);

    PCHK(LWARN, m, malloc(sizeof(*m)));
    if(!m){
        r = 1;
        goto err;
    }

    m->memb = memb;
    ICHK(LWARN, r, ll_add_front(h->memb, (void *)&m));

    r = 0;

    //grp_print(h);
    grp_isok(h);
err:
    return r;
}

int grp_get_memb(struct nn_grp *h, void **memb, int max)
{
    struct nn_grp_memb *m;
    int i = 0;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while(m=ll_next(h->memb, &iter)){
        // randomize?
        memb[i++] = m;
    }

    return 0;
}

int grp_ismemb(struct nn_grp *h, struct nn_node *memb)
{
    int r = 1;
    struct nn_grp_memb *m;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while(m=ll_next(h->memb, &iter)){
        if(m->memb == memb){
            r = 0;
            break;
        }
    }

    return r;
}

int grp_rem_memb(struct nn_grp *h, struct nn_node *memb)
{
    int r = 1;
    struct nn_grp_memb *m;
    void *iter;

    grp_isok(h);

    iter = NULL;
    while(m=ll_next(h->memb, &iter)){
        if(m->memb == memb){
            ICHK(LWARN, r, ll_rem(h->memb, m));
            r = 0;
            grp_isok(h);
            grp_print(h);
            break;
        }
    }


    return r;
}

