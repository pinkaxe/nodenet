
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"

#include "node_net/pkt.h"

struct nn_pkt {
    struct nn_node *src;
    void *data;
    int data_len;
    void *pdata;
    int refcnt;
    struct nn_pkt_conf *conf;
    buf_free_cb_f buf_free_cb;
    //uint32_t seq_len;
    mutex_t mutex;
    cond_t cond;
};

static int pkt_lock(struct nn_pkt *pkt);
static int pkt_unlock(struct nn_pkt *pkt);

struct nn_pkt *pkt_init(struct nn_node *src, void *data, int data_len, void
        *pdata, int sendto_no, int sendto_type, int sendto_id, buf_free_cb_f
        buf_free_cb)
{
    struct nn_pkt *pkt;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, pkt, malloc(sizeof(*pkt)));
    if(!pkt){
        goto err;
    }
    pkt->src = src;
    pkt->data = data;
    pkt->data_len = data_len;
    pkt->pdata = pdata;
    pkt->refcnt = 0;
    pkt->conf = NULL;
    pkt->buf_free_cb = buf_free_cb;


    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        pkt_free(pkt);
        pkt = NULL;
        goto err;
    }
    conf->sendto_no = sendto_no;
    conf->sendto_type = sendto_type;
    conf->sendto_id = sendto_id;

    pkt->conf = conf;

    // FIXME: err checking
    mutex_init(&pkt->mutex, NULL);
    cond_init(&pkt->cond, NULL);

err:

    return pkt;
}

struct nn_pkt *pkt_clone(struct nn_pkt *pkt)
{
    struct nn_pkt *clone;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->src = pkt->src;
    clone->data = pkt->data;
    clone->data_len = pkt->data_len;
    clone->pdata = pkt->pdata;
    clone->refcnt = pkt->refcnt;
    clone->buf_free_cb = pkt->buf_free_cb;
    clone->conf = NULL;

    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        pkt_free(clone);
        clone = NULL;
        goto err;
    }
    conf->sendto_no = pkt->conf->sendto_no;
    conf->sendto_type = pkt->conf->sendto_type;
    conf->sendto_id = pkt->conf->sendto_id;

    clone->conf = conf;

err:
    return clone;
}

int pkt_free(struct nn_pkt *pkt)
{

    if(pkt){

        pkt_lock(pkt);

        if(--pkt->refcnt <= 0){
            if(pkt->buf_free_cb){
                pkt->buf_free_cb(pkt->pdata, pkt->data);
            }

            if(pkt->conf){
                free(pkt->conf);
            }

            pkt_unlock(pkt);

            cond_destroy(&pkt->cond);
            mutex_destroy(&pkt->mutex);

            free(pkt);
            pkt = NULL;
        }

        if(pkt){
            pkt_unlock(pkt);
        }
    }


    return 0;
}


struct nn_node *pkt_get_src(struct nn_pkt *pkt)
{
    struct nn_node *n;

    pkt_lock(pkt);

    n = pkt->src;

    pkt_unlock(pkt);

    return n;
}

void *pkt_get_data(struct nn_pkt *pkt)
{
    void *r;

    pkt_lock(pkt);

    r = pkt->data;

    pkt_unlock(pkt);

    return r;
}

int pkt_get_data_len(struct nn_pkt *pkt)
{
    int r;

    pkt_lock(pkt);

    r = pkt->data_len;

    pkt_unlock(pkt);

    return r;
}

void *pkt_get_pdata(struct nn_pkt *pkt)
{
    void *r;

    pkt_lock(pkt);

    r = pkt->pdata;

    pkt_unlock(pkt);
}

int pkt_set_src(struct nn_pkt *pkt, struct nn_node *n)
{
    int r = 0;

    pkt_lock(pkt);

    pkt->src = n;

    pkt_unlock(pkt);

    return r;
}

int pkt_set_refcnt(struct nn_pkt *pkt, int refcnt)
{
    pkt_lock(pkt);

    pkt->refcnt = refcnt;

    pkt_unlock(pkt);

    return 0;
}

int pkt_get_refcnt(struct nn_pkt *pkt)
{
    int r;

    pkt_lock(pkt);

    r = pkt->refcnt;

    pkt_unlock(pkt);

    return r;
}

int pkt_inc_refcnt(struct nn_pkt *pkt, int inc)
{
    int r;

    pkt_lock(pkt);

    pkt->refcnt += inc;
    r = pkt->refcnt;

    pkt_unlock(pkt);

    return r;
}

/* 0 -> can reuse */
int pkt_reuse(struct nn_pkt *pkt)
{
    if(pkt->refcnt == 1){
        return 0;
    }else{
        return 1;
    }
}

static int pkt_lock(struct nn_pkt *pkt)
{
    mutex_lock(pkt);
    return 0;
}

static int pkt_unlock(struct nn_pkt *pkt)
{
    mutex_unlock(pkt);
}
