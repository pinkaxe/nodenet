

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"

#include "code_net/io.h"

struct cn_io_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/elem/all */
    int sendto_id;  /* depend on send_to_type grp_id/elem_id */
};

struct cn_io_cmd {
    enum cn_elem_cmd id;
    void *pdata;
    int data_no;
    struct cn_io_conf *conf;
};

struct cn_io_data {
    void *data;
    int data_no;
    struct cn_io_conf *conf;
};


struct cn_io_cmd *io_cmd_init(enum cn_elem_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id)
{
    struct cn_io_cmd *cmd;
    struct cn_io_conf *conf;

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
        io_cmd_free(cmd);
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

int io_cmd_free(struct cn_io_cmd *cmd)
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
struct cn_io_data_req {
    struct code_elem *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

struct cn_io_cmd_req {
    cmd_id;
}

/*
struct cn_io *io_init()
{
    int r;
    int err;
    struct cn_io *io;

    PCHK(LWARN, io, calloc(1, sizeof(*io)));
    if(!io){
    io    goto err;
    }
    DBG_STRUCT_INIT(io);
}

int io_free(struct cn_io *io)
{
}
*/

int io_add_data_req(struct code_net *net, struct code_elem *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

int io_add_cmd_req(struct code_net *net, struct code_elem *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

static void io_ctrl_thread(void *arg)
{
    // loop and call code_io_ctrl_default for each req
}

int cn_io_ctrl_default(struct code_io_data_req *req)
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
