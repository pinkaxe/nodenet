
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/pkt.h"

struct nn_pkt_conf {
    int sendto_no;
    int sendto_type;
    int sendto_id;
};

struct nn_pkt {
    int id;
    void *pdata;
    int data_no;
    struct nn_pkt_conf *conf;
    //uint32_t seq_no;
};

struct nn_pkt *pkt_init(int id, void *pdata, int data_no, int sendto_no, int
        sendto_type, int sendto_id)
{
    struct nn_pkt *pkt;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, pkt, malloc(sizeof(*pkt)));
    if(!pkt){
        goto err;
    }
    pkt->id = id;
    pkt->pdata = pdata;
    pkt->data_no = data_no;
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

struct nn_pkt *pkt_clone(struct nn_pkt *pkt)
{
    struct nn_pkt *clone;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->id = pkt->id;
    clone->pdata = pkt->pdata;
    clone->data_no = pkt->data_no;
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

