
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<assert.h>

#include "util/log.h"
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

struct ll_iter{
    struct ll_node *curr;
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

static int _ll_rem(struct ll *h, struct ll_node *n);
static struct ll_node *ll_next_node(struct ll *h,  void **iter);

/* free's all the nodes, not what they point to though */
static int ll_free_nodes(struct ll *h)
{
    void *iter;
    int r;
    int fail = 0;
    struct ll_node *n;

    iter = NULL;
    while((n=ll_next_node(h, &iter))){
        ICHK(LWARN, r, _ll_rem(h, n));
        if(r) fail++;
    }

    return fail;
}

int ll_free(struct ll *h)
{
    int r = 0;
    assert(h);

    ICHK(LWARN, r, ll_free_nodes(h));

    free(h);

    return r;
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

struct ll_iter *ll_iter_init(struct ll *h)
{
    struct ll_iter *iter;

	PCHK(LWARN, iter, calloc(1, sizeof *iter));
    if(!iter){
        goto err;
	}

    iter->curr = h->start;

err:
    return iter;
}

int ll_iter_free(struct ll_iter *iter)
{
    free(iter);
    return 0;
}


int ll_iter_next(struct ll_iter *iter, void **data)
{
    int r;

    if(iter->curr){
        *data = iter->curr->data;
        iter->curr = iter->curr->next;
        r = 0;
    }else{
        *data = NULL;
        r = 1;
    }
    return r;
}

#if 0

int ll_next4(struct ll *h, void **data, void **iter)
{
    int r = 0;
    struct ll_node *n;

    if(*iter){
        /* middle */
        n = *iter;
        *data = n->data;
        *iter = n->next;
    }else{
        /* first */
        n = h->start;
        if(n){
            *data = n->data;
            *iter = n->next;
        }
    }

    return r;
}

#endif

#if 0
/* return ll_node */
static struct ll_node *ll_next_node(struct ll *h,  void **iter)
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
#endif

/* return ll_node */
static struct ll_node *ll_next_node(struct ll *h,  void **iter)
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
    }
    return r;
}

#if 0
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
#endif

void *ll_prev(void **iter)
{
    //assert(curr);
    //return curr->prev;
    return NULL;
}
