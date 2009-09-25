
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"
#include "util/ll.h"

#include "types.h"
#include "net.h"

struct cn_net_memb {
    struct cn_elem *memb;
    struct link link;
};

struct cn_net {
    struct ll *memb;
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

    n->memb = ll_init(struct cn_net_memb, link, &err);
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
        ll_free(n->memb);
    }

    free(n);
    return 0;
}

int net_add_memb(struct cn_net *n, struct cn_elem *e)
{
    net_isvalid(n);

    return ll_add_front(n->memb, e);
}

int net_rem_memb(struct cn_net *n, struct cn_elem *e)
{
    net_isvalid(n);

    ll_rem(n->memb, e);
    return 0;
}

int net_ismemb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    void *track;
    struct cn_net_memb *curr;

    assert(n);
    printf("%p\n", n->memb);

    ll_foreach(n->memb, curr, track){
        if(curr->memb == e){
            r = 0;
            break;
        }
    }

    return r;
}

int net_print(struct cn_net *n)
{
    void *track;
    struct cn_net_memb *curr;
    int r = 0;
    int c;

    printf("\n-- net->elem --: %p\n\n", n);
    c = 0;

    ll_foreach(n->memb, curr, track){
        printf("zee\n");
        printf("p:%p\n", curr->memb);
        c++;
    }

    printf("total: %d\n\n", c);

    return 0;
}
