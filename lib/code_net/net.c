
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "util/log.h"
#include "util/ll2.h"

#include "types.h"
#include "io.h"
#include "net.h"

struct cn_net_memb {
    struct cn_elem *memb;
};

struct cn_net {
    struct ll2 *memb;
    struct ll2 *cmd_req;
    io_cmd_req_cb_t io_cmd_req_cb;
    struct ll2 *data_req;
    io_data_req_cb_t io_data_req_cb;
};


int net_isvalid(struct cn_net *n)
{
    assert(n->memb);
}

struct cn_net *net_init()
{
    int err;
    struct cn_net *n;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }

    PCHK(LWARN, n->memb, ll2_init());
    if(!n->memb){
        net_free(n);
        n = NULL;
        goto err;
    }

    net_isvalid(n);
err:
    return n;
}

int net_free(struct cn_net *n)
{
    int r = 0;
    net_isvalid(n);

    if(n->memb){
        ICHK(LWARN, r, ll2_free(n->memb));
    }

    free(n);
    return r;
}

int net_add_memb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    net_isvalid(n);

    PCHK(LWARN, nm, malloc(sizeof(*nm)));
    if(!nm){
        goto err;
    }

    nm->memb = e;
    ICHK(LWARN, r, ll2_add_front(n->memb, (void **)&nm));

err:
    return r;
}

int net_rem_memb(struct cn_net *n, struct cn_elem *e)
{
    int r;

    net_isvalid(n);

    ICHK(LWARN, r, ll2_rem(n->memb, e));

    return r;
}

int net_ismemb(struct cn_net *n, struct cn_elem *e)
{
    int r = 1;
    struct cn_net_memb *nm;
    void *iter;

    assert(n);

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
    struct cn_net_memb *nm;
    int r = 0;
    int c;
    void *iter;

    printf("\n-- net->elem --: %p\n\n", n);
    c = 0;

    iter = NULL;
    ll2_each(n->memb, nm, iter){
    //while((nm = ll2_next(n->memb, &iter))){
        printf("zee\n");
        printf("p:%p\n", nm->memb);
        c++;
    }

    printf("total: %d\n\n", c);

    return 0;
}

int net_set_cmd_cb(struct cn_net *n, io_cmd_req_cb_t cb)
{
}

int net_add_cmd_req(struct cn_net *n, struct cn_io_cmd_req *req)
{
}
