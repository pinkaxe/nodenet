
#ifndef UTIL_LL_H__
#define UTIL_LL_H__

#include<stddef.h> // offsetof

struct ll;
struct ll_iter;

struct ll *ll_init();
int ll_free(struct ll *h);

int ll_add_front(struct ll *h, void **data);
int ll_rem(struct ll *h, void *data);

//void *ll_first(struct ll *h, void **iter);
//int ll_next(struct ll_iter *iter, void **data);
//int ll_next2(struct ll *h, void **res, void **iter);

//void *ll_get_end(struct ll *h);
void *ll_prev(void **iter);

struct ll_iter *ll_iter_init(struct ll *h);
int ll_iter_next(struct ll_iter *iter, void **data);
int ll_iter_free(struct ll_iter *iter);

#if 0

#define ll_each(r, h, nm, iter) \
    for(iter=NULL,r = ll_next2(h, (void **)(&nm), (void **)&iter); \
        !r; \
        r = ll_next2(h, (void **)(&nm), (void **)(&iter))) \

#define ll_each(r, h, nm, iter) \
    for(iter=NULL,r = ll_next2(h, (void **)(&nm), (void **)&iter); \
        !r; \
        r = ll_next2(h, (void **)(&nm), (void **)(&iter))) \

#define ll_foreach(h, curr, track) \
	for(curr=ll_get_start(h),track=ll_next(h, curr); \
			curr && ((track=ll_next(h, curr)) || 1) ; \
            curr=track)

int ll_add(struct ll *h, void *node, void *new);
void *ll_rem(struct ll *h, void *node);

//void *ll_rem_match(struct ll *h, void *match, bool (*cb)(void *match, void
//            *item))

//bool ll_find(struct ll *h, void *node);

int ll_add_front(struct ll *h, void *new);
int ll_add_end(struct ll *h, void *new);
void *ll_rem_end(struct ll *h);

//void ll_del(void *item);
void *ll_next(struct ll *h, void *curr);


#define ll_foreach(h, curr, track) \
	for(curr=ll_get_start(h),track=ll_next(h, curr); \
			curr && ((track=ll_next(h, curr)) || 1) ; \
            curr=track)

#define ll_foreach(start, curr, track) \
	for(curr=start,track=curr->conn.next; \
			curr && ((track=curr->conn.next) || 1) ; \
            curr=track)
#endif

#endif
