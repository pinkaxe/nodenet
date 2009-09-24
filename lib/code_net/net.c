
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "arch/thread.h"

#include "util/log.h"
#include "util/dpool.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "net.h"

struct cn_net_elem {
    struct cn_elem *elem;
    struct link link;
};

struct cn_net {
    struct cn_net_elem *elem;
};
 
int net_isvalid(struct cn_net *n)
{
}

struct cn_net *net_init()
{
    struct cn_net *n = NULL;

    n = malloc(sizeof(*n));
    if(!n){
        goto err;
    }

    net_isvalid(n);
err:
    return n;
}

int net_free(struct cn_net *n)
{
    net_isvalid(n);
    free(n);
    return 0;
}

int net_add_memb(struct cn_net *h, struct cn_elem *e)
{
    /* add to e->neth */
    /* add to h->elemh */
}

int net_rem_memb(struct cn_net *h, struct cn_elem *e)
{
    /* rem from e->neth */
    /* rem from h->elemh */
}
