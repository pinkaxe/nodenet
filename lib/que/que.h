#ifndef UTIL_QUE_H__
#define UTIL_QUE_H__

struct que;

struct que *que_init(int len);
int que_free(struct que *h);
int que_add(struct que *h, void *item);
void *que_get(struct que *h, struct timespec *ts);
void *que_set_get_cb(struct que *h, void (*get_cb)(void *p));

#endif
