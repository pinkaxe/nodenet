
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/pkt.h"

struct nn_pkt {
    struct nn_node *src;
    void *data;
    int data_len;
    void *pdata;
    struct nn_io_conf *conf;
    //uint32_t seq_len;
};

struct nn_node *pkt_get_src(struct nn_pkt *pkt)
{
    return pkt->src;
}

void *pkt_get_data(struct nn_pkt *pkt)
{
    return pkt->data;
}

int pkt_get_data_len(struct nn_pkt *pkt)
{
    return pkt->data_len;
}

void *pkt_get_pdata(struct nn_pkt *pkt)
{
    return pkt->pdata;
}

struct nn_pkt *pkt_init(struct nn_node *src, void *data, int data_len,
        void *pdata, int sendto_no, int sendto_type, int sendto_id)
{
    struct nn_pkt *pkt;
    struct nn_io_conf *conf;

    PCHK(LWARN, pkt, malloc(sizeof(*pkt)));
    if(!pkt){
        goto err;
    }
    pkt->src = src;
    pkt->data = data;
    pkt->data_len = data_len;
    pkt->pdata = pdata;
    pkt->conf = NULL;

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

err:

    return pkt;
}

int pkt_set_src(struct nn_pkt *pkt, struct nn_node *n)
{
    int r = 0;

    pkt->src = n;

    return r;
}



struct nn_pkt *pkt_clone(struct nn_pkt *pkt)
{
    struct nn_pkt *clone;
    struct nn_io_conf *conf;

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->src = pkt->src;
    clone->data = pkt->data;
    clone->data_len = pkt->data_len;
    clone->pdata = pkt->pdata;
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
        if(pkt->conf){
            free(pkt->conf);
        }
        free(pkt);
    }

    return 0;
}
