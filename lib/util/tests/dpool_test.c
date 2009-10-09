
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdint.h>

#include "util/log.h"
#include "util/bitmap.h"
#include "util/dpool.h"

struct item {
    int x;
};

int main(int argc, char **argv)
{
    int i;
    struct dpool *h;
    void *p, *p2;

    for(;;){
        h = dpool_create(sizeof(struct item), 4, DPOOL_OPT_ALLOC);
        if(!h){
            printf("!! err\n");
        }

        for(i=0; i < 1024; i++){
            p = dpool_get_buf(h);
            p2 = dpool_get_buf(h);
            printf("!! got buf: %p\n", p);
            printf("!! got buf: %p\n", p2);
            sleep(1);
        //    printf("!! return buf: %d\n", dpool_ret_buf(h, p));
        //    printf("!! return buf: %d\n", dpool_ret_buf(h, p2));
        }

        dpool_free(h);
    }

    return 0;
}
