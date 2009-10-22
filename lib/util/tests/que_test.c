
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include "util/que.h"

static struct que *q;
static char *str[] = {"abc", "def", "ghi"};
static int str_no = sizeof(str) / sizeof(str[0]);

void *adder(void *arg)
{
    int r;
    int i;

    for(;;){
        for(i=0; i < str_no; i++){
            while((r=que_add(q, str[i]))){
                //printf("full\n");
                usleep(10);
            }
            //printf("add %p(%d)\n", str[i], r);
        }
    }
    return NULL;
}

void *remover(void *arg)
{
    char *curr;
    int i;

    for(;;){
        for(i=0; i < str_no; i++){
            while(!(curr=que_get(q, NULL))){
                usleep(10);
            }
            //printf("get %p: %p\n", curr, str[i]);
            assert(curr == str[i]);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t tid;

    q = que_init(2);

    pthread_create(&tid, NULL, adder, NULL);
    pthread_detach(tid);

    pthread_create(&tid, NULL, remover, NULL);
    pthread_detach(tid);

    for(;;){
        sleep(1);
    }

    que_free(q);

    return 0;
}
