
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
    void *p[1024];

 //   int max 1024;
 //   int min = 1;
 //   int loopc;

    for(;;){
        h = dpool_create(sizeof(struct item), 1, DPOOL_OPT_ALLOC);
        if(!h){
            printf("!! err\n");
            exit(-1);
        }

//        for(loopc=0; loopc < currc; 
        /* get some for use */
        for(i=0; i < 4; i++){
            p[i] = dpool_get_buf(h);
        }

        /* return some */
        for(i=1; i < 3; i++){
            printf("!! return buf: %d\n", dpool_ret_buf(h, p[i]));
        }

        for(i=0; i < 1; i++){
            p[i] = dpool_get_buf(h);
        }
        sleep(1);

        dpool_free(h);
    }

    return 0;
}
