#ifndef __LL_H__
#define __LL_H__

#include<stddef.h> // offsetof

struct ll;

struct link{
	void *prev;
	void *next;
};

struct ll *__ll_init(int offset, int *err);

#define ll_init(type, link, err)\
    __ll_init(offsetof(type, link), err)

void ll_free(struct ll *h);

void *ll_get_start(struct ll *h);

int ll_add(struct ll *h, void *elem, void *new);
void *ll_rem(struct ll *h, void *elem);
//bool ll_find(struct ll *h, void *elem);

int ll_add_front(struct ll *h, void *new);
int ll_add_end(struct ll *h, void *new);
void *ll_rem_end(struct ll *h);

//void ll_del(void *item);
void *ll_next(struct ll *h, void *curr);


#define ll_foreach(h, curr, track) \
	for(curr=ll_get_start(h),track=ll_next(h, curr); \
			curr && ((track=ll_next(h, curr)) || 1) ; \
            curr=track)

//#define ll_foreach(start, curr, track) \
//	for(curr=start,track=curr->link.next; \
//			curr && ((track=curr->link.next) || 1) ; \
//            curr=track)

#endif
