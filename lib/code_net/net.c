
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"
#include "util/ll2.h"

#include "types.h"
#include "net.h"

struct cn_net_memb {
    struct cn_elem *memb;
};

struct cn_net {
    struct ll2 *memb;
};


int net_isvalid(struct cn_net *n)
{
    assert(n->memb);
}

struct cn_net *net_init()
{
    int err;
    struct cn_net *n = NULL;

    n = malloc(sizeof(*n));
    if(!n){
        goto err;
    }

    n->memb = ll2_init();
    if(!n->memb){
        printf("err!\n");
        net_free(n);
        goto err;
    }

    net_isvalid(n);
err:
    return n;
}

int net_free(struct cn_net *n)
{
    net_isvalid(n);

    if(n->memb){
        ll2_free(n->memb);
    }

    free(n);
    return 0;
}

int net_add_memb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    net_isvalid(n);

    nm = malloc(sizeof(*nm));
    if(!nm){
        printf("err!!!\n");
        goto err;
    }

    nm->memb = e;
    r = ll2_add_front(n->memb, (void **)&nm);

err:
    return r;
}

int net_rem_memb(struct cn_net *n, struct cn_elem *e)
{
    net_isvalid(n);

    ll2_rem(n->memb, e);
    return 0;
}

int net_ismemb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    void *iter;

    assert(n);
    printf("%p\n", n->memb);

    iter = NULL;
    while((nm=ll2_next(n->memb, &iter))){
        if(nm->memb == e){
            r = 0;
            break;
        }
    }

    return r;
}

int net_print(struct cn_net *n)
{
    void *track;
    struct cn_net_memb *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\n-- net->elem --: %p\n\n", n);
    c = 0;

    iter = NULL;
    while((nm=ll2_next(n->memb, &iter))){
        printf("zee\n");
        printf("p:%p\n", nm->memb);
        c++;
    }

    printf("total: %d\n\n", c);

    return 0;
}
