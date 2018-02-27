#ifndef UTIL_BITMAP_H__
#define UTIL_BITMAP_H__

/* no locking,  depending on app lib user have to do locking */

struct bitmap;

struct bitmap *bitmap_init(uint32_t bits);
void bitmap_free(struct bitmap *h);

int bitmap_get_bit(struct bitmap *h, int start, int offset);
int bitmap_ret_bit(struct bitmap *h, int bit_no);

bool bitmap_bit_inuse(struct bitmap *h, int bit_no);

void bitmap_print(struct bitmap *h);

#endif
