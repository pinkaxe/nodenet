
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "util/ll2.h"

#include "types.h"
#include "grp.h"


struct cn_grp_memb {
    struct cn_elem *memb;
};

struct cn_grp {
    struct ll2 *membh;
    int id;
    int memb_no;
};

struct cn_grp *grp_init(int id)
{
    int err;
    struct cn_grp *g;
    g = calloc(1, sizeof(*g));
    if(!g){
        goto err;
    }

    g->membh = ll2_init();

    if(!g->membh){
        printf("goto err: %d\n", err);
        grp_free(g);
        goto err;
    }

err:

    return g;
}

int grp_free(struct cn_grp *g)
{
    if(g){
        if(g->membh){
            ll2_free(g->membh);
        }
        free(g);
    }

    return 0;
}


int grp_add_memb(struct cn_grp *h, struct cn_elem *memb)
{
    int r;
    struct cn_grp_memb *m;

    m = malloc(sizeof(*m));
    if(!m){
        r = 1;
        goto err;
    }

    m->memb = memb;
    ll2_add_front(h->membh, (void *)&m);

    r = 0;

err:
    return r;
}

int grp_get_memb(struct cn_grp *h, void **memb, int max)
{
    struct cn_grp_memb *m;
    int i = 0;
    void *iter;

    iter = NULL;
    while(m=ll2_next(h->membh, &iter)){
        // randomize?
        memb[i++] = m;
    }

    return 0;
}

bool grp_is_memb(struct cn_grp *h, void *memb)
{
    bool r = false;
    struct cn_grp_memb *m;
    void *iter;

    iter = NULL;
    while(m=ll2_next(h->membh, &iter)){
        if(m->memb == memb){
            r = true;
            break;
        }
    }

    return r;
}

int grp_rem_memb(struct cn_grp *h, void *memb)
{
    int r = 1;
    struct cn_grp_memb *m;
    void *iter;

    iter = NULL;
    while(m=ll2_next(h->membh, &iter)){
        if(m->memb == memb){
            ll2_rem(h->membh, m);
            r = 0;
            break;
        }
    }

    return r;
}

