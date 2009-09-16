#ifndef __CYBUF_H__
#define __CYBUF_H__

struct cybuf;

struct cybuf *cybuf_init(int len);
int cybuf_free(struct cybuf *h);
int cybuf_add(struct cybuf *h, void *item);
void *cybuf_get(struct cybuf *h);
void *cybuf_set_get_cb(struct cybuf *h, void (*get_cb)(void *p));

#endif
