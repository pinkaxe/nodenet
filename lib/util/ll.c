
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<assert.h>

#include "util/log.h"

#include "ll.h"

struct ll_node {
	struct ll_node *prev;
    void *data;
	struct ll_node *next;
};

struct ll {
	struct ll_node *start;
	struct ll_node *end;
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
	struct ll_node *n;

	PCHK(LWARN, n, calloc(1, sizeof *n));
    if(!n){
        goto err;
	}

    if(!h->start){
        n->prev = NULL;
        n->data = *data;
        n->next = NULL;
        h->start = n;
    }else{
        n->prev = NULL;
        n->data = *data;
        n->next = h->start;
        h->start->prev = n;
        h->start = n;
    }

    h->c++;

err:
	return 0;
}

static int _ll_rem(struct ll *h, struct ll_node *n)
{
    int r;

    if(n->prev){
        n->prev->next = n->next;
    }else{
        /* first */
        h->start = n->next;
    }

    if(n->next){
        n->next->prev = n->prev;
    }else{
        /* last */
        h->end = n->prev;
    }

    h->c--;
    free(n);
    r = 0;

    return r;
}

int ll_rem(struct ll *h, void *data)
{
    int r = 1;
    struct ll_node *n;
    void *iter = NULL;

    n = h->start;
    while(n){
        if(n->data == data){
            r = _ll_rem(h, n);
            break;
        }
        n = n->next;
    }

    return r;
}



void *ll_next(struct ll *h,  void **iter)
{
    void *r = NULL;
    struct ll_node *_iter;

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
    struct ll_node *_iter;

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
