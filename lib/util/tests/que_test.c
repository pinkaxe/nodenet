
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
                usleep(100000);
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
    struct timespec ts = {0, 0};


    for(;;){
        for(i=0; i < str_no; i++){
            while(!(curr=que_get(q, &ts))){
                usleep(100000);
            }
            //printf("get %p: %p\n", curr, str[i]);
            assert(curr == str[i]);
            sleep(1);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    struct timespec ts = {0, 0};
    char *curr;
    pthread_t tid;

    q = que_init(9);

    while(!(curr=que_get(q, &ts))){
        usleep(100000);
    }

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
