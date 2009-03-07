#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

#include "util/bitmap.h"

int main(int argc, char **argv)
{
    struct bitmap *b;

    for(;;){
        b = bitmap_create(8);
        if(!b){
            printf("fail\n");
        }

        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        printf("got %d\n", bitmap_get_bit(b, 0, 8));
        bitmap_print(b);

        printf("got %d\n", bitmap_ret_bit(b, 0));
        printf("got %d\n", bitmap_ret_bit(b, 1));
        printf("got %d\n", bitmap_ret_bit(b, 2));
        printf("got %d\n", bitmap_ret_bit(b, 3));
        printf("got %d\n", bitmap_ret_bit(b, 4));
        printf("got %d\n", bitmap_ret_bit(b, 5));
        printf("got %d\n", bitmap_ret_bit(b, 6));
        printf("got %d\n", bitmap_ret_bit(b, 7));
        bitmap_print(b);

        printf("got %d\n", bitmap_get_bit(b, 7, 8));
        bitmap_print(b);

        bitmap_print(b);

        bitmap_free(b);
    }

    return 0;
}
