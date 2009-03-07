
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdint.h>
#include<errno.h>

#include "util/log.h"
#include "util/bitmap.h"
#include "util/dpool.h"

struct dpool{
    size_t bufsize;
    size_t max_no;
    struct dpool_buf **bufs;
    struct bitmap *bitmap;
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

    h->bufs = calloc(max_no, sizeof(struct dpool_buf *));
    if(!h->bufs){
        dpool_free(h);
        goto err;
    }
    
    h->bufsize = bufsize;
    h->max_no = max_no;
     
    h->bitmap = bitmap_create(max_no);
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
        free(h);
    } 


}


struct dpool_buf *dpool_get_buf(struct dpool *h)
{
    void *buf = NULL;
    int n;
    
    /* get the number of a free buf */
    n = bitmap_get_bit(h->bitmap, 0, 4);
    if(n == -1){
        errno = -ENONEAVAIL;
    }else{
        /* return the free buf */
        buf = h->bufs[n];
    }

    return buf;

}

int dpool_ret_buf(struct dpool *h, struct dpool_buf *buf)
{
    return bitmap_ret_bit(h->bitmap, buf->id);
}

