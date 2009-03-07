#ifndef BITMAP_H
#define BITMAP_H

struct bitmap;

struct bitmap *bitmap_create(int bits);
void bitmap_free(struct bitmap *h);

int bitmap_get_bit(struct bitmap *h, int start, int offset);
int bitmap_ret_bit(struct bitmap *h, int bit_no);

bool bitmap_bit_inuse(struct bitmap *h, int bit_no);

void bitmap_print(struct bitmap *h);

#endif
