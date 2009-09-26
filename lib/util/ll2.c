
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<assert.h>

#include "util/log.h"

#include "ll2.h"

struct ll2_elem {
	struct ll2_elem *prev;
    void *data;
	struct ll2_elem *next;
};

struct ll2 {
	struct ll2_elem *start;
	struct ll2_elem *end;
    int c;
};

struct ll2 *ll2_init()
{
	struct ll2 *h;

	PCHK(LWARN, h, calloc(1, sizeof(*h)));
    if(!h){
        goto err;
	}

err:
	return h;

}

int ll2_free(struct ll2 *h)
{
    assert(h);
    free(h);
}

int ll2_add_front(struct ll2 *h, void **data)
{
	struct ll2_elem *e;

	PCHK(LWARN, e, calloc(1, sizeof *e));
    if(!e){
        goto err;
	}

    if(!h->start){
        e->prev = NULL;
        e->data = *data;
        e->next = NULL;
        h->start = e;
    }else{
        e->prev = NULL;
        e->data = *data;
        e->next = h->start;
        h->start->prev = e;
        h->start = e;
    }

    h->c++;

err:
	return 0;
}

int ll2_rem(struct ll2 *h, void *data)
{
    int r = 1;
    struct ll2_elem *e;
    void *iter = NULL;

    e = h->start;
    while(e){
        if(e->data == data){

            if(e->prev){
                e->prev->next = e->next;
            }else{
                /* first */
                h->start = e->next;
            }

            if(e->next){
                e->next->prev = e->prev;
            }else{
                /* last */
                h->end = e->prev;
            }

            h->c--;
            r = 0;
            break;
        }
        e = e->next;
    }

    return r;
}



void *ll2_next(struct ll2 *h, void **iter)
{
    void *r = NULL;
    struct ll2_elem *_iter;

    if(!(*iter)){
        /* first */
        *iter = h->start;
        _iter = *iter;
        if(_iter){
            r = _iter->data;
        }
        goto end;
    }

    _iter = *iter;
    if(_iter->next){
        /* got next */
        r = _iter->next->data;
        *iter = _iter->next;
    }

end:
    return r;
}

void *ll2_prev(void **iter)
{
    //assert(curr);
    //return curr->prev;
}
