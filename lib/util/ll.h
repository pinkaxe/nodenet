
#ifndef UTIL_LL_H__
#define UTIL_LL_H__


struct ll;
struct ll_iter;

struct ll *ll_init();
int ll_free(struct ll *h);

int ll_add_front(struct ll *h, void **data);
int ll_rem(struct ll *h, void *data);

void *ll_prev(void **iter);

struct ll_iter *ll_iter_init(struct ll *h);
int ll_iter_next(struct ll_iter *iter, void **data);
int ll_iter_free(struct ll_iter *iter);

#endif
