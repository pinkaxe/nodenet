
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

//#include "node_net/icmd.h"

struct nn_pkt_conf {
    int sendto_no;
    int sendto_type;
    int sendto_id;
};

struct nn_icmd {
    int id;
    void *pdata;
    int data_no;
    struct nn_pkt_conf *conf;
    //uint32_t seq_no;
};

struct nn_icmd *icmd_init(int id, void *pdata, int data_no, int sendto_no, int
        sendto_type, int sendto_id)
{
    struct nn_icmd *icmd;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, icmd, malloc(sizeof(*icmd)));
    if(!icmd){
        goto err;
    }
    icmd->id = id;
    icmd->pdata = pdata;
    icmd->data_no = data_no;
    icmd->conf = NULL;

    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        icmd_free(icmd);
        icmd = NULL;
        goto err;
    }
    conf->sendto_no = sendto_no;
    conf->sendto_type = sendto_type;
    conf->sendto_id = sendto_id;

    icmd->conf = conf;

err:

    return icmd;
}

struct nn_icmd *icmd_clone(struct nn_icmd *icmd)
{
    struct nn_icmd *clone;
    struct nn_pkt_conf *conf;

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->id = icmd->id;
    clone->pdata = icmd->pdata;
    clone->data_no = icmd->data_no;
    clone->conf = NULL;

    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        icmd_free(clone);
        clone = NULL;
        goto err;
    }
    conf->sendto_no = icmd->conf->sendto_no;
    conf->sendto_type = icmd->conf->sendto_type;
    conf->sendto_id = icmd->conf->sendto_id;

    clone->conf = conf;

err:
    return clone;
}

int icmd_free(struct nn_icmd *icmd)
{
    if(icmd){
        if(icmd->conf){
            free(icmd->conf);
        }
        free(icmd);
    }

    return 0;
}

