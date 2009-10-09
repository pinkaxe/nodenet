
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<assert.h>

#include "util/bitmap.h"

struct bitmap {
    uint32_t bits_no;
    size_t bytes_no;
    uint8_t *map;
};


struct bitmap *bitmap_init(uint32_t bits)
{
    struct bitmap *h;

    h = malloc(sizeof(struct bitmap));
    if(!h){
        goto err;
    }

    h->bits_no = bits;
    h->bytes_no = (bits / 8) + 1;

    h->map = calloc(h->bytes_no, sizeof(uint8_t));
    if(!h->map){
        bitmap_free(h);
        h = NULL;
        goto err;
    }

err:
    return h;
}


void bitmap_free(struct bitmap *h)
{
    if(h->map){
        free(h->map);
    }
    if(h){
        free(h);
    }
}


int bitmap_get_bit(struct bitmap *h, int start, int offset)
{
    int r, i, byte_no, bit_offset, end_offset;
    bool found = false;
    int bits_valid = 8;

    byte_no = start / 8;
    bit_offset = start % 8;

    end_offset = h->bits_no % 8; /* how many bits filled in last byte */

    if(byte_no == h->bytes_no){
        bits_valid = end_offset;
    }

    for(i=bit_offset; i < bits_valid; i++){
        /* find a free bit */
        if((h->map[byte_no] & (1 << i)) == 0){
            /* set to one */
           h->map[byte_no] |= (1 << i);
           found = true;
           break;
        }

        /* check if we should go to the next byte */
        if(i >= 7){
            byte_no++;
            if(byte_no == h->bytes_no){
                bits_valid = end_offset;
            }
            i = -1;
        }
    }

    if(!found){
        r = -1;
    }else{
        r = (byte_no * 8) + i;
    }

    return r;
}


bool bitmap_bit_inuse(struct bitmap *h, int bit_no)
{
    bool r;
    int byte_no, bit_offset;


    byte_no = bit_no / 8;
    bit_offset = bit_no % 8;

    if((h->map[byte_no] & (1 << bit_offset)) != 0){
        r = true;
    }else{
        r = false;
    }

    return r;
}


int bitmap_ret_bit(struct bitmap *h, int bit_no)
{
    int r = -1;
    int byte_no, bit_offset;

    if(bit_no >= h->bits_no){
        r = -2;
        goto err;
    }

    if(!bitmap_bit_inuse(h, bit_no)){
        r = -1;
        printf("!!! x\n");
        goto err;
    }

    byte_no = bit_no / 8;
    bit_offset = bit_no % 8;

    h->map[byte_no] = h->map[byte_no] & ~(1 << bit_offset);
    r = 0;

err:
    return r;
}


void bitmap_print(struct bitmap *h)
{
    size_t i;

    for(i=0; i < h->bits_no; i++){
        printf("bit no %d: %d\n", i, bitmap_bit_inuse(h, i));
    }
        /*
    for(i=0; i < h->bytes_no; i++){
        printf("byte no %d: %x\n", i, h->map[i]);
    }
    */

}
