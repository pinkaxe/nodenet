
#include<stdlib.h>
#include<stdio.h>
#include<stddef.h>

#include "util/log.h"
#include "util/ll.h"

struct node {
    int y;
	int x;
};

int main()
{
    int r;
    struct ll *h;
	struct node *new, *n;
    //void *iter = NULL;
    struct ll_iter *iter;
	int j;
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
        while((!ll_iter_next(iter, &n))){
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

      //  iter = NULL;
      //  while((n=ll_next(h, &iter))){
      //      //printf("**%d\n", n->x);
      //  }

      //  int r;
      //  ll_each(r, h, n, iter){
      //      //printf("xx **%d\n", n->x);
      //      ll_rem(h, n);
      //      free(n);
      //  }

        //sleep(2);
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
}
