
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>

#include "util/log.h"
#include "util/ll.h"
#include "arch/thread.h"



struct ll {
	void *start;
	void *end;
    int offset; /* offset of link in struct */
    int c;
    mutex_t mutex;
};


struct ll *__ll_init(int offset, int *err)
{
	struct ll *h;

	if(!(h = malloc(sizeof *h))){
        *err = errno;
        goto err;
	}

    mutex_init(&h->mutex, NULL);

	h->offset = offset;
	h->start = NULL;
	h->end = NULL;
	h->c = 0;

    printf("init done\n");
err:
	return h;
}


void ll_free(struct ll *h)
{
    int r;
    r = mutex_destroy(&h->mutex);
    free(h);
}


int ll_add_front(struct ll *h, void *new) 
{
	struct link *new_link; // = new + h->offset;
    struct link *start_link;

    mutex_lock(&h->mutex);

    new_link = new + h->offset;
    start_link = h->start + h->offset;

    if(!h->start){
        h->start = h->end = new;
        new_link->prev = NULL;
        new_link->next = NULL;
    }else{
        new_link->prev = NULL;
        new_link->next = h->start;
        start_link->prev = new;
        h->start = new;
        h->c++;
    }

    mutex_unlock(&h->mutex);

	return 0;
}

int ll_add_end(struct ll *h, void *new) 
{
	struct link *new_link;
    struct link *end_link;

    mutex_lock(&h->mutex);

    new_link = new + h->offset;
    end_link = h->end + h->offset;

    if(!h->start){
        h->start = h->end = new;
        new_link->prev = NULL;
        new_link->next = NULL;
    }else{
        end_link->next = new;
        new_link->prev = h->end;
        new_link->next = NULL;
        h->end = new;
        h->c++;
    }

    mutex_unlock(&h->mutex);

    return 0;
}

#if 0

int ll_rem_front(struct ll *h, void **item) 
{
	*item = h->start - h->offset;

    if(h->start){
        h->end = h->end->prev;
    }
    /* last item? */
    if(!h->start){
        h->end= NULL;
    }else{
        h->start->prev = NULL;
    }

    return 0;
}


int ll_get_front(struct ll *h, void **item) 
{
	*item = h->start - h->offset;

    return 0;
}

#define xll_rem_end(h, type, elem) \
{ \
    void *entry \
    entry = list_entry(h->end, type, elem); \
    _ll_rem_end(); \
}
#endif

void *ll_rem_end(struct ll *h)
{
	void *item = h->end;
    struct link *end_link = h->end + h->offset;

    mutex_lock(&h->mutex);

    if(h->end){
        h->end = end_link->prev;
    }
    /* last item? */
    if(!h->end){
        h->start = NULL;
        //errno = -ENODATA;
    }else{
        end_link->next = NULL;
    }

    mutex_unlock(&h->mutex);

    return item;
}


/*
int ll_get_end(struct ll *h, void **item) 
{
	*item = h->end - h->offset;
    return 0;
}
*/


/*
void ll_del(struct ll *h, void *item)
{
	struct link *item_link = item;
	struct link *prev = item_link->prev;
	struct link *next = item_link->next;

	if(prev)
		prev->next = NULL;
	if(next)
		next->prev = NULL;

	if(prev && next){
		prev->next = next;
		next->prev = prev;
	}
}

void *ll_next(struct ll *h, void *curr)
{
	struct link *link = curr;
	return link->next;
}

*/

/*
struct ll *ll_init(size_t size)
{
	struct ll *ll;

	if(!(ll = malloc(sizeof *ll))){
		return NULL;
	}

	ll->p = NULL;
	ll->n = NULL;

	return ll;
}

int ll_insert()
{
}


int ll_remove()
{
}
*/
