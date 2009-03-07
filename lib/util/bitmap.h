#ifndef BITMAP_H
#define BITMAP_H

struct bitmap;

extern struct bitmap *bitmap_create(size_t bits);
extern void bitmap_free(struct bitmap *h);

extern int bitmap_get_bit(struct bitmap *h, int start, int offset);
extern int bitmap_ret_bit(struct bitmap *h, int bit_no);

extern bool bitmap_bit_inuse(struct bitmap *h, int bit_no);

extern void bitmap_print(struct bitmap *h);

#endif
