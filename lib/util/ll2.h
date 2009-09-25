
#ifndef __LL2_H__
#define __LL2_H__

#include<stddef.h> // offsetof

struct ll2;
struct ll2_iter;

struct ll2 *ll2_init();
int ll2_free(struct ll2 *h);

int ll2_add_front(struct ll2 *h, void **data);
int ll2_rem(struct ll2 *h, void *data);

//void *ll2_first(struct ll2 *h, void **iter);
void *ll2_next(struct ll2 *h, void **iter);

//void *ll2_get_end(struct ll2 *h);
void *ll2_prev(void **iter);













#if 0
#define ll_foreach(h, curr, track) \
	for(curr=ll_get_start(h),track=ll_next(h, curr); \
			curr && ((track=ll_next(h, curr)) || 1) ; \
            curr=track)

int ll_add(struct ll *h, void *elem, void *new);
void *ll_rem(struct ll *h, void *elem);

//void *ll_rem_match(struct ll *h, void *match, bool (*cb)(void *match, void
//            *item))

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

#endif
