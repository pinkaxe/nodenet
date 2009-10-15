#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "link.h"

struct link {
    enum link_state state; /* not needed but good for validation */
    void *from;
    void *to;
};


struct link *link_init()
{
    int r = 0;
    struct link *cn;

    PCHK(LWARN, cn, calloc(1, sizeof(*cn)));
    if(!cn){
        goto err;
    }


err:
    return cn;
}

/* p -> pointer to free */
static _link_free(struct link *cn, void **p)
{
    int r = 0;

    if(*p){
        cn->state = LINK_STATE_DEAD;
    }
    *p = NULL;

    if(!cn->to && !cn->from){
        free(cn);
        r = 1;
    }

    return r;
}

int link_free_from(struct link *cn)
{
    return _link_free(cn, &cn->from);

}

int link_free_to(struct link *cn)
{
    return _link_free(cn, &cn->to);
}

void *link_get_from(struct link *cn)
{
    return cn->from;
}

void *link_get_to(struct link *cn)
{
    return cn->to;
}

enum link_state link_get_state(struct link *cn)
{
    return cn->state;
}

int link_set_from(struct link *cn, void *from)
{
    assert(from);

    cn->from = from;
    return 0;
}

int link_set_to(struct link *cn, void *to)
{
    assert(to);

    cn->to = to;
    if(!to){
        cn->state = LINK_STATE_DEAD;
    }
    return 0;
}

int link_set_state(struct link *cn, enum link_state state)
{
    cn->state = state;
    return 0;
}


/*
int main(int argc, char const* argv[])
{
    struct link *cn;
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
