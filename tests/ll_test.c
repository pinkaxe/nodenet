
#include<stdlib.h>
#include<stdio.h>
#include<stddef.h>

#include "util/log.h"
#include "util/ll.h"

struct elem {
    int y;
	int x;
	struct link link;
};

int main()
{
    int r;
    int err;
    struct ll *h;
	struct elem *new, *curr; 
	struct elem *start, *track;
	int j;
	int i;

    for(;;){

        h = ll_init(struct elem, link, &err);
        if(!h){
            r = err;
            goto err;
        }

		for(i=0; i < 10; i++){
			new = malloc(sizeof *new);
			new->x = i;
			r = ll_add_front(h, new);
            if(r){
                goto err;
            }
		}

        track = NULL;
        //start = new;
        ll_foreach(h, curr, track) {
            printf("*%d\n", curr->x);
            if(curr->x < 6){
                ll_rem(h, curr);
                free(curr);
            }
        }


        track = NULL;
        ll_foreach(h, curr, track) {
            printf("**%d\n", curr->x);
        }
        sleep(2);

#if 0
		for(i=0; i < 10; i++){
			curr = ll_rem_end(h);
            if(curr){
                printf("%d:%d\n", i, curr->x);
                free(curr);
            }
            //free(curr);
        }

		for(i=0; i < 10; i++){
			new = malloc(sizeof *new);
			new->x = i;
			r = ll_add_end(h, new);
            if(r){
                goto err;
            }
		}

		for(i=0; i < 10; i++){
			curr = ll_rem_end(h);
            if(curr){
                printf("-- %d:%d\n", i, curr->x);
                free(curr);
            }
            //free(curr);
        }
#endif
        ll_free(h);
	}

err:
	return r;
}
