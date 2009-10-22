
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
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

    for(;;){
        h = dpool_create(sizeof(struct item), 6, DPOOL_OPT_ALLOC);
        if(!h){
            L(LCRIT, "dpool create failed");
        }

        /* get some buffers for use */
        for(i=0; i < 4; i++){
            p[i] = dpool_get_buf(h);
            L(LNOTICE, "dpool get buf res: %p\n", p[i]);
        }

        /* return some */
        for(i=1; i < 3; i++){
            L(LNOTICE, "dpool return buf res: %d\n", dpool_ret_buf(h, p[i]));
        }

        /* get some again */
        for(i=0; i < 1; i++){
            p[i] = dpool_get_buf(h);
        }

        dpool_free(h);
    }

    return 0;
}
