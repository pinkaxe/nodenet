#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<assert.h>

#include "util/bitmap.h"

/* - loop bits_no = min .. max
 *      - create a bitmap with size bits_no
 *      - loop bit_empty = 0 .. bits_no
 *          - get bits, keeping bits_empty empty
 *          - check right amount is filled
 *          - ret half
 *          - check right amount is filled
 *      - free
 */
int test_brute(int min, int max)
{
    int i, c;
    struct bitmap *b;
    int bits_no, bits_fill;
    int bits_empty, bits_ret;

    for(bits_no=min; bits_no <= max; bits_no++){
        for(bits_empty=0; bits_empty < bits_no; bits_empty++){
            b = bitmap_init(bits_no);
            bits_fill = bits_no - bits_empty;
            if(!b){
                printf("fail\n");
                exit(-1);
            }

            /* get bits */
            for(i=0; i < bits_fill; i++){
                assert(bitmap_get_bit(b, 0, max) != -1);
            }

            /* check the right amount is filled */
            for(c=0, i=0; i < bits_no; i++){
                if(bitmap_bit_inuse(b, i)){
                    c++;
                }
            }
            assert(c == bits_fill);

            /* return half */
            bits_ret = bits_fill / 2;
            for(i=0; i < bits_ret; i++){
                assert(bitmap_ret_bit(b, i) == 0);
            }

            /* check the right amount is filled */
            for(c=0, i=0; i < bits_no; i++){
                if(bitmap_bit_inuse(b, i)){
                    c++;
                }
            }
            assert(c == bits_fill - bits_ret);

            bitmap_free(b);
        }
    }

    return 0;
}


int main(int argc, char **argv)
{
    return test_brute(1, 1024);
}
