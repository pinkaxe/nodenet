
#include<stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include<assert.h>

#include "util/log.h"
#include "sys/thread.h"

struct que {
    int head, tail, len;
    void **pp;
    void (*get_cb)(void *p);
    mutex_t mutex;
    cond_t cond;
};

struct que *que_init(int len)
{
    struct que *h;

    h = calloc(1, sizeof(struct que));
    if(!h){
        goto end;
    }

    h->len = len;
    h->pp = calloc(len, sizeof(void *));
    if(!h->pp){
        que_free(h);
        h = NULL;
        goto end;
    }

    mutex_init(&h->mutex, NULL);
    cond_init(&h->cond, NULL);

end:
    return h;
}

int que_free(struct que *h)
{
    if(h->pp){
        free(h->pp);
    }

    //mutex_free(&h->mutex);
    //cond_free(&h->cond);

    if(h){
        free(h);
    }

    return 1;
}

int que_add(struct que *h, void *item) 
{
    int r = 0;
    assert(h);

    mutex_lock(&h->mutex);

    if((h->head == h->len && h->tail == 0) || 
            h->head == h->tail - 1){
        r = 1;
        goto err;
    }

    h->pp[h->head] = item;

    if(++h->head >= h->len){
        h->head = 0;
    }

    cond_signal(&h->cond);

err:
    mutex_unlock(&h->mutex);

    return r;
}

void *que_get(struct que *h, struct timespec *ts)
{
    int waitr;
    int e = 0;
    void *r = NULL;
    struct timespec timeout;
    long long nsec;

    mutex_lock(&h->mutex);

    while(h->tail == h->head){
        if(ts){
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += ts->tv_sec;
            nsec = timeout.tv_nsec + ts->tv_nsec;
            //printf("-- %ld, %ld\n", nsec, nsec / 1000000);
            timeout.tv_sec += (nsec / 1000000000);
            timeout.tv_nsec = (nsec % 1000000000);

            //printf("--start wait: %ld, %ld\n", timeout.tv_sec, timeout.tv_nsec);
            waitr = cond_timedwait(&h->cond, &h->mutex, &timeout);
            printf("--end wait: %d\n", waitr);
            if(waitr == ETIMEDOUT){
            //printf("--timeout\n");
                e = ETIMEDOUT;
                goto timeout;
            }
        }else{
            cond_wait(&h->cond, &h->mutex);
        }
    }

    r = h->pp[h->tail];

    if(++h->tail >= h->len){
        h->tail = 0;
    }

timeout:
    mutex_unlock(&h->mutex);

    if(e) errno = e;
    return r;
}

void *que_set_get_cb(struct que *h, void (*get_cb)(void *p)) 
{
    h->get_cb = get_cb;
}

/*
int main(int argc, char **argv)
{
    int i;
    struct que *h;
    char *str0 = "abc";
    char *str1 = "---";

    while(1){
        h = que_init(6);
        for(i=0; i < 2; i++){
            que_add(h, str0);     
            que_add(h, str1);     
            que_add(h, str0);     
            que_add(h, str1);     
            que_add(h, str0);     
            que_add(h, str1);     
            printf("** %s\n", (char *)que_get(h));
            printf("** %s\n", (char *)que_get(h));
            printf("** %s\n", (char *)que_get(h));
            printf("** %s\n", (char *)que_get(h));
            printf("** %s\n", (char *)que_get(h));
            printf("** %s\n", (char *)que_get(h));
        }
        que_free(h);
    }

    return 0;
}
*/

