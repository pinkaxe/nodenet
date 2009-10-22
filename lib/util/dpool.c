
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdint.h>
#include<errno.h>

#include "sys/thread.h"

#include "util/log.h"
#include "util/bitmap.h"
#include "util/dpool.h"


struct dpool{
    size_t bufsize;
    size_t max_no;
    struct dpool_buf **bufs;
    struct bitmap *bitmap;
    mutex_t mutex;
    cond_t cond;
};

struct dpool *dpool_create(size_t bufsize, size_t max_no, int opt)
{
    int i;
    struct dpool *h = NULL;

    h = calloc(1, sizeof(struct dpool));
    if(!h){
        dpool_free(h);
        goto err;
    }
    mutex_init(&h->mutex, NULL);
    cond_init(&h->cond, NULL);

    h->bufs = calloc(max_no, sizeof(struct dpool_buf *));
    if(!h->bufs){
        dpool_free(h);
        goto err;
    }

    h->bufsize = bufsize;
    h->max_no = max_no;

    /* use a bitmap to keep track of which bufs used/unused */
    h->bitmap = bitmap_init(max_no);
    if(!h->bitmap){
        dpool_free(h);
        goto err;
    }

    //if(opt & DPOOL_OPT_ALLOC){
        for(i=0; i < (int)h->max_no; i++){
            h->bufs[i] = malloc(sizeof(struct dpool_buf));
            if(!h->bufs[i]){
                dpool_free(h);
                goto err;
            }

            h->bufs[i]->id = i;
            h->bufs[i]->data = malloc(h->bufsize);
            h->bufs[i]->ref_cnt = 1;

            if(!h->bufs[i]->data){
                dpool_free(h);
                goto err;
            }
        }
    //}

err:
    return h;
}

void dpool_free(struct dpool *h)
{
    int i;
    for(i=0; i < (int) h->max_no; i++){
        free(h->bufs[i]->data);
        free(h->bufs[i]);
    }

    if(h->bitmap){
        bitmap_free(h->bitmap);
    }

    if(h->bufs){
        free(h->bufs);
    }


    if(h){
        cond_destroy(&h->cond);
        mutex_destroy(&h->mutex);
        free(h);
    }

}


struct dpool_buf *dpool_get_buf(struct dpool *h)
{
    void *buf = NULL;
    int n;

    mutex_lock(&h->mutex);

    /* get the number of a free buf */
    n = bitmap_get_bit(h->bitmap, 0, 4);
    printf("!! got %d\n", n);

    if(n == -1){
        errno = -ENONEAVAIL;
    }else{
        /* return the free buf */
        buf = h->bufs[n];
    }

    mutex_unlock(&h->mutex);

    return buf;

}

struct dpool_buf *dpool_get_filled_buf(struct dpool *h)
{
    void *buf = NULL;
    int n;

    mutex_lock(&h->mutex);

    /* get the number of a free buf */
    n = bitmap_get_bit(h->bitmap, 0, 4);
    if(n == -1){
        errno = -ENONEAVAIL;
    }else{
        h->bufs[n]->ref_cnt = 1;
        /* return the free buf */
        buf = h->bufs[n];
    }

    mutex_unlock(&h->mutex);

    return buf;

}

int dpool_ret_buf(struct dpool *h, struct dpool_buf *buf)
{
    int r = -1;

    mutex_lock(&h->mutex);

    if(buf->ref_cnt <= 1){
        r = bitmap_ret_bit(h->bitmap, buf->id);
    }else{
        buf->ref_cnt--;
    }

    mutex_unlock(&h->mutex);

    return r;
}

