#ifndef UTIL_SBUF_H__
#define UTIL_SBUF_H__

struct sbuf *sbuf_init(size_t size, int *n);
void sbuf_free(struct sbuf *b);

int sbuf_grow(struct sbuf *b, size_t size);
int sbuf_append(struct sbuf *b, const char *fmt, ...);
int sbuf_insert(struct sbuf *b, size_t pos, const char *fmt, ...);
int sbuf_replace(struct sbuf *b, const char *old, const char *new, int num);
char *sbuf_get(struct sbuf *b);

char *sbuf_find(const char *buf, const char *str);
size_t sbuf_count(const char *buf, const char *str);

#endif
