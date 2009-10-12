
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/cmd.h"


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


#if 0
struct nn_io_data_req {
    struct code_node *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

struct nn_cmd_req {
    cmd_id;
}

/*
struct nn_io *io_init()
{
    int r;
    int err;
    struct nn_io *io;

    PCHK(LWARN, io, calloc(1, sizeof(*io)));
    if(!io){
    io    goto err;
    }
    DBG_STRUCT_INIT(io);
}

int io_free(struct nn_io *io)
{
}
*/

int io_add_data_req(struct code_router *router, struct code_node *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

int io_add_cmd_req(struct code_router *router, struct code_node *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

static void io_ctrl_thread(void *arg)
{
    // loop and call code_io_ctrl_default for each req
}

int nn_io_ctrl_default(struct code_io_data_req *req)
{
    // check req and send appropriately
    switch(req->type){
        case ALL:
            ll_foreach(req->from->out_link_queh, curr, track){
                code_tx_data(curr->link, buf, len);
            }
            break;
        case NO:
            ll_foreach(req->from->out_link_queh, curr, track){
                code_tx_data(curr->link, buf, len);
                c++;
            }
            break;
        case GRP_ALL:
            break;
        case GRP_NO:
            break;
        default:
            break;
    }
    // remove back pointers
}
#endif
