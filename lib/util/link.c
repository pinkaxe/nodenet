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
    struct nn_link *cn;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }

    mutex_init(&cn->mutex, NULL);

err:
    return cn;
}

/* both link_free_from and link_free_to have to be called for
 * it to be free'd */
static int link_free(struct nn_link *cn)
{
    /* free only if both from and to points to nothing */
    mutex_destroy(&cn->mutex);
    free(cn);
}

int link_free_to(struct nn_link *cn)
{
    cn->to = NULL;

    if(!cn->to && !cn->from){
        link_free(cn);
    }else{
        cn->state = NN_LINK_STATE_DEAD;
    }

    return 0;
}

int link_free_from(struct nn_link *cn)
{
    cn->from = NULL;

    if(!cn->to && !cn->from){
        link_free(cn);
    }else{
        cn->state = NN_LINK_STATE_DEAD;
    }

    return 0;
}

void *link_get_from(struct nn_link *cn)
{
    return cn->from;
}

void *link_get_to(struct nn_link *cn)
{
    return cn->to;
}

int link_set_from(struct nn_link *cn, void *from)
{
    assert(from);

    cn->from = from;
    return 0;
}

int link_set_to(struct nn_link *cn, void *to)
{
    assert(to);

    cn->to = to;
    if(!to){
        cn->state = NN_LINK_STATE_DEAD;
    }
    return 0;
}



/*
int main(int argc, char const* argv[])
{
    struct nn_link *cn;
    int from, to;

    while(1){

        cn = link_init();
        link_set_from(cn, &from);
        link_set_to(cn, &to);

        assert(link_get_from(cn) == &from);
        assert(link_get_to(cn) == &to);

        link_free_to(cn);
        link_free_from(cn);
    }

    return 0;
}
*/
