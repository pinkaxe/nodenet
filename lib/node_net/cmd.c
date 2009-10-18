
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/cmd.h"

struct nn_cmd {
    enum nn_cmd_cmd id;
    void *pdata;
    int data_no;
    struct nn_io_conf *conf;
    //uint32_t seq_no;
};

enum nn_cmd_cmd cmd_get_id(struct nn_cmd *cmd)
{
    return cmd->id;
}

struct nn_cmd *cmd_init(enum nn_cmd_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id)
{
    struct nn_cmd *cmd;
    struct nn_io_conf *conf;

    PCHK(LWARN, cmd, malloc(sizeof(*cmd)));
    if(!cmd){
        goto err;
    }
    cmd->id = id;
    cmd->pdata = pdata;
    cmd->data_no = data_no;
    cmd->conf = NULL;

    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        cmd_free(cmd);
        cmd = NULL;
        goto err;
    }
    conf->sendto_no = sendto_no;
    conf->sendto_type = sendto_type;
    conf->sendto_id = sendto_id;

    cmd->conf = conf;

err:

    return cmd;
}

struct nn_cmd *cmd_clone(struct nn_cmd *cmd)
{
    struct nn_cmd *clone;
    struct nn_io_conf *conf;

    PCHK(LWARN, clone, malloc(sizeof(*clone)));
    if(!clone){
        goto err;
    }
    clone->id = cmd->id;
    clone->pdata = cmd->pdata;
    clone->data_no = cmd->data_no;
    clone->conf = NULL;

    PCHK(LWARN, conf, malloc(sizeof(*conf)));
    if(!conf){
        cmd_free(clone);
        clone = NULL;
        goto err;
    }
    conf->sendto_no = cmd->conf->sendto_no;
    conf->sendto_type = cmd->conf->sendto_type;
    conf->sendto_id = cmd->conf->sendto_id;

    clone->conf = conf;

err:
    return clone;
}

int cmd_free(struct nn_cmd *cmd)
{
    if(cmd){
        if(cmd->conf){
            free(cmd->conf);
        }
        free(cmd);
    }

    return 0;
}
