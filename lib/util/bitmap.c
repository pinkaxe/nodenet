
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>

struct bitmap {
    size_t bytes_no;
    unsigned int *map;
};


struct bitmap *bitmap_create(size_t bits)
{
    struct bitmap *h;
    size_t bytes_no;   

    h = malloc(sizeof(struct bitmap));
    if(!h){
        goto err;  
    }
    
    h->bytes_no = bits / 8;

    h->map = calloc(bytes_no, sizeof(unsigned int));
    if(!h->map){
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
    int r, i, byte_no, bit_offset;
    bool found = false;

    byte_no = start / 8;
    bit_offset = start % 8;

    for(i=bit_offset; i < 8; i++){
        /* find a free bit */
        if((h->map[byte_no] & (1 << i)) == 0){
            /* set to one */
           h->map[byte_no] |= (1 << i);
           found = true;
           break;
        }

        /* check if we should go to the next byte */
        if(i >= 8){
            byte_no++;
            bit_offset = 0;
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

    if(!bitmap_bit_inuse(h, bit_no)){
        r = -1;
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
    int i;

    for(i=0; i < (int) h->bytes_no; i++){
        printf("byte no %d: %x\n", i, h->map[i]);
    }

}
