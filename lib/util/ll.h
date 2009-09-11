#ifndef __LL_H__
#define __LL_H__

struct ll;

struct link{
	void *prev;
	void *next;
};


#define ll_init(type, link, err) \
    __ll_init(offsetof(type, link), err)

struct ll *__ll_init(int offset, int *err);
void ll_free(struct ll *h);
int ll_add_front(struct ll *h, void *new);
int ll_add_end(struct ll *h, void *new);
void *ll_rem_end(struct ll *h);

//void ll_del(void *item);
//void *ll_next(void *curr);
//#define ll_foreach(start, curr, track) \
//	for(curr=start,track=curr->link.next; \
//			curr && ((track=curr->link.next) || 1) ; \
//			curr=track)

#endif
