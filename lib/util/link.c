
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../sys/thread.h"
#include "log.h"
#include "link.h"

enum nn_link_state {
    NN_LINK_STATE_ALIVE = 0x01,
    NN_LINK_STATE_DEAD
};

struct nn_link {
    enum nn_link_state state; /* not needed but good for validation */
    void *from;
    void *to;
    mutex_t mutex;
};


struct nn_link *link_init()
{
    int r = 0;
    struct nn_link *l;

    PCHK(LWARN, l, calloc(1, sizeof(*l)));
    if(!l){
        goto err;
    }

    mutex_init(&l->mutex, NULL);

err:
    return l;
}

/* both link_free_from and link_free_to have to be called for
 * it to be free'd */
static int link_free(struct nn_link *l)
{
    /* free only if both from and to points to nothing */
    mutex_destroy(&l->mutex);
    free(l);
}

int link_free_to(struct nn_link *l)
{
    l->to = NULL;

    if(!l->to && !l->from){
        link_free(l);
    }else{
        l->state = NN_LINK_STATE_DEAD;
    }

    return 0;
}

int link_free_from(struct nn_link *l)
{
    l->from = NULL;

    if(!l->to && !l->from){
        link_free(l);
    }else{
        l->state = NN_LINK_STATE_DEAD;
    }

    return 0;
}

void *link_get_from(struct nn_link *l)
{
    return l->from;
}

void *link_get_to(struct nn_link *l)
{
    return l->to;
}

int link_set_from(struct nn_link *l, void *from)
{
    assert(from);

    l->from = from;
    return 0;
}

int link_set_to(struct nn_link *l, void *to)
{
    assert(to);

    l->to = to;
    if(!to){
        l->state = NN_LINK_STATE_DEAD;
    }
    return 0;
}



/*
int main(int argc, char const* argv[])
{
    struct nn_link *l;
    int from, to;

    while(1){

        l = link_init();
        link_set_from(l, &from);
        link_set_to(l, &to);

        assert(link_get_from(l) == &from);
        assert(link_get_to(l) == &to);

        link_free_to(l);
        link_free_from(l);
    }

    return 0;
}
*/
