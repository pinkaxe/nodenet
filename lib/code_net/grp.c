
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "util/ll.h"

#include "types.h"
#include "grp.h"

struct cn_grp_memb {
    struct cn_elem *memb;
    struct link *link;
};

struct cn_grp {
    struct ll *membh;
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

    g->membh = ll_init(struct cn_grp_memb, link, &err);
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
            ll_free(g->membh);
        }
        free(g);
    }

    return 0;
}

int code_add_to_grp(struct cn_elem *h, struct cn_grp *grp)
{
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
    ll_add_front(h->membh, m);

    code_add_to_grp(memb, h);

    r = 0;

err:
    return r;
}

int grp_get_memb(struct cn_grp *h, void **memb, int max)
{
    struct cn_grp_memb *m;
    void *track;
    int i = 0;

    ll_foreach(h->membh, m, track){
        // randomize?
        memb[i++] = m;
    }

    return 0;
}

bool grp_is_memb(struct cn_grp *h, void *memb)
{
    bool r = false;
    struct cn_grp_memb *m;
    void *track;

    ll_foreach(h->membh, m, track){
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
    void *track;

    ll_foreach(h->membh, m, track){
        if(m->memb == memb){
            ll_rem(h->membh, m);
            r = 0;
            break;
        }
    }

    return r;
}

