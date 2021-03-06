
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "sys/thread.h"

#include "log/log.h"

#include "nodenet/pkt.h"


#if 0

int pkt_ev_inc_refcnt(struct nn_pkt *pkt, int inc)
{
    int r;

    pkt_lock(pkt);

    pkt->refcnt += inc;
    r = pkt->refcnt;

    pkt_unlock(pkt);

    return r;
}
#endif




struct nn_pkt {
    enum nn_pkt_state state;
    struct nn_node *src;
    struct nn_chan *dest_chan;
    int dest_no;
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


struct nn_pkt *pkt_init(struct nn_node *src, struct nn_chan *ch, int dest_no,
        void *data, int data_len, void *pdata, buf_free_cb_f buf_free_cb)
{
    struct nn_pkt *pkt;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, pkt, malloc(sizeof(*pkt)));
    if(!pkt){
        goto err;
    }
    pkt->src = src;
    pkt->dest_chan = ch; /* destination grp_id */
    pkt->dest_no = dest_no;
    pkt->data = data;
    pkt->data_len = data_len;
    pkt->pdata = pdata;
    pkt->refcnt = 1;
    pkt->conf = NULL;
    pkt->buf_free_cb = buf_free_cb;

    pkt->state = PKT_STATE_N_INIT;


    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        pkt_free(pkt);
        pkt = NULL;
        goto err;
    }
    //conf->sendto_no = sendto_no;

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

    pkt_lock(pkt);

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->src = pkt->src;
    clone->dest_chan = pkt->dest_chan;
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
    //conf->sendto_type = pkt->conf->sendto_type;
   // conf->sendto_id = pkt->conf->sendto_id;

    clone->conf = conf;

err:
    pkt_unlock(pkt);
    return clone;
}

int pkt_free(struct nn_pkt *pkt)
{

    if(pkt){

        pkt_lock(pkt);

        //printf("!!!! pkt_free: %d\n", pkt->refcnt);
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

int pkt_set_state(struct nn_pkt *pkt, enum nn_pkt_state state)
{

    pkt_lock(pkt);

    pkt->state = state;

    pkt_unlock(pkt);

    return 0;
}

int pkt_cancelled(struct nn_pkt *pkt)
{

#if 0
    int r = 0;
    pkt_lock(pkt);

    if(pkt->state == PKT_STATE_CANCELLED){
        /* free it for this pkt user */
        pkt_unlock(pkt);
        pkt_free(pkt);
        r = 1;
    }else{
        pkt_unlock(pkt);
    }
#endif

    return 0;
}

enum nn_pkt_state pkt_get_state(struct nn_pkt *pkt)
{
    enum nn_pkt_state state;
    pkt_lock(pkt);

    state = pkt->state; 

    pkt_unlock(pkt);

    return state;
}



struct nn_node *pkt_get_src(struct nn_pkt *pkt)
{
    struct nn_node *n;

    pkt_lock(pkt);

    n = pkt->src;

    pkt_unlock(pkt);

    return n;
}

struct nn_chan *pkt_get_dest_chan(struct nn_pkt *pkt)
{
    struct nn_chan *chan;

    pkt_lock(pkt);

    chan = pkt->dest_chan;

    pkt_unlock(pkt);

    return chan;
}

int pkt_get_dest_no(struct nn_pkt *pkt)
{
    int r;

    pkt_lock(pkt);

    r = pkt->dest_no;

    pkt_unlock(pkt);

    return r;
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

    return  r;
}

int pkt_set_src(struct nn_pkt *pkt, struct nn_node *n)
{
    int r = 0;

    pkt_lock(pkt);

    pkt->src = n;

    pkt_unlock(pkt);

    return r;
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

int pkt_dec_refcnt(struct nn_pkt *pkt, int dec)
{
    int r;

    pkt_lock(pkt);

    pkt->refcnt -= dec;
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
    mutex_lock(&pkt->mutex);
    return 0;
}

static int pkt_unlock(struct nn_pkt *pkt)
{
    mutex_unlock(&pkt->mutex);
    return 0;
}
