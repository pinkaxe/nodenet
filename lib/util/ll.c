
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<assert.h>

#include "util/log.h"

#include "ll.h"

struct ll_elem {
	struct ll_elem *prev;
    void *data;
	struct ll_elem *next;
};

struct ll {
	struct ll_elem *start;
	struct ll_elem *end;
    int c;
};

struct ll *ll_init()
{
	struct ll *h;

	PCHK(LWARN, h, calloc(1, sizeof(*h)));
    if(!h){
        goto err;
	}

err:
	return h;

}

int ll_free(struct ll *h)
{
    assert(h);
    free(h);
}

int ll_add_front(struct ll *h, void **data)
{
	struct ll_elem *e;

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

static int _ll_rem(struct ll *h, struct ll_elem *e)
{
    int r;

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
    free(e);
    r = 0;

    return r;
}

int ll_rem(struct ll *h, void *data)
{
    int r = 1;
    struct ll_elem *e;
    void *iter = NULL;

    e = h->start;
    while(e){
        if(e->data == data){
            r = _ll_rem(h, e);
            break;
        }
        e = e->next;
    }

    return r;
}



void *ll_next(struct ll *h,  void **iter)
{
    void *r = NULL;
    struct ll_elem *_iter;

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

int ll_next2(struct ll *h, void **res, void **iter)
{
    int r = 1;
    struct ll_elem *_iter;

    if(!(*iter)){
        /* first */
        *iter = h->start;
        _iter = *iter;
        if(_iter){
            *res = _iter->data;
            if(*res){
                r = 0;
            }
        }
        goto end;
    }

    _iter = *iter;
    if(_iter->next){
        /* got next */
        *res = _iter->next->data;
        if(*res){
            r = 0;
        }
        *iter = _iter->next;
    }

end:
    return r;
}

void *ll_prev(void **iter)
{
    //assert(curr);
    //return curr->prev;
}
