#include <stdio.h>
#include <string.h>
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
    struct link *l;

    PCHK(LWARN, l, calloc(1, sizeof(*l)));
    if(!l){
        goto err;
    }


err:
    return l;
}

/* p -> pointer to free */
static int _link_free(struct link *l, void **p)
{
    int r = 0;

    if(*p){
        l->state = LINK_STATE_DEAD;
    }
    *p = NULL;

    if(!l->to && !l->from){
        free(l);
        r = 1;
    }

    return r;
}

int link_free_from(struct link *l)
{
    return _link_free(l, &l->from);

}

int link_free_to(struct link *l)
{
    return _link_free(l, &l->to);
}

void *link_get_from(struct link *l)
{
    return l->from;
}

void *link_get_to(struct link *l)
{
    return l->to;
}

enum link_state link_get_state(struct link *l)
{
    return l->state;
}

int link_set_from(struct link *l, void *from)
{
    assert(from);

    l->from = from;
    return 0;
}

int link_set_to(struct link *l, void *to)
{
    assert(to);

    l->to = to;
    if(!to){
        l->state = LINK_STATE_DEAD;
    }
    return 0;
}

int link_set_state(struct link *l, enum link_state state)
{
    l->state = state;
    return 0;
}


/*
int main(int argc, char const* argv[])
{
    struct link *l;
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
