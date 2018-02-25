
#include<stdlib.h>
#include<stdio.h>
#include<stddef.h>

#include "util/log.h"
#include "ll.h"

struct node {
    int y;
	int x;
};

int main()
{
    int r;
    struct ll *h;
	struct node *new, *n;
    struct ll_iter *iter;
	int i;

    for(;;){

        h = ll_init();
        if(!h){
            r = 1;
            goto err;
        }

		for(i=0; i < 10; i++){
			new = malloc(sizeof *new);
			new->x = i;
			r = ll_add_front(h, (void **)&new);
            if(r){
                goto err;
            }
		}

        iter = ll_iter_init(h);
        while((!ll_iter_next(iter, (void **)&n))){
            printf("*%d\n", n->x);
            ll_rem(h, n);
            free(n);
        }
        ll_iter_free(iter);

        ll_free(h);
        //sleep(5);
    }

err:
	return r;
}
