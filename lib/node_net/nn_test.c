
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/types.h"
#include "node_net/cmd.h"
#include "node_net/nn.h"

#define ok(x){ \
    assert(x); \
}

#define GRP0 0

static struct nn_node *n0, *n1, *n2;

int input_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    //j
    //nn_io_write(n, 0, b, rt, cleanup_cb);
    //L(LWARN, "loop\rt");
    return 0;
}

int process_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    printf("!! process\n");
    //n->add_output_data();
    return 0;
}

int process_lproc_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    printf("!! process lproc_\n");
    return 0;
}

int main(int argc, char *argv)
{
    int i;
    struct nn_router *rt0;
    struct nn_node *n[1024];
    struct nn_grp *g0;
    struct nn_cmd *cmd;
    struct nn_io_data *data;
    struct nn_io_conf *conf;

    while(1){
        rt0 = nn_router_init();
        ok(rt0);

        g0 = nn_grp_init(GRP0);
        ok(g0);

        n0 = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT, input_node, NULL);
        ok(n0);

        //while(1){
        //sleep(5);
        nn_add_node_to_router(n0, rt0);
        nn_add_node_to_grp(n0, g0);
        //nn_rem_node_from_router(n0, rt0);


        n1 = nn_node_init(NN_NODE_TYPE_THREAD, 0, process_node, NULL);
        ok(n1);

        nn_add_node_to_router(n1, rt0);
        nn_add_node_to_grp(n1, g0);


        n2 = nn_node_init(NN_NODE_TYPE_LPROC, 0, process_lproc_node, NULL);
        ok(n2);

        //nn_add_node_to_router(n2, rt0);
        nn_add_node_to_grp(n2, g0);

        nn_node_run(n0);
        nn_node_run(n1);
        //nn_node_run(n2);
        //nn_node_run(n1);

        int i;
        for(i=0; i < 300; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT, process_node, NULL);
            nn_add_node_to_router(n[i], rt0);
            nn_add_node_to_grp(n[i], g0);
            nn_node_run(n[i]);
        }


        //nn_router_set_cmd_cb(rt0, io_cmd_req_cb);
        //nn_router_set_data_cb(rt0, io_data_req_cb);
        nn_router_run(rt0);

        for(i = 0; i < 128; i++){

            cmd = cmd_init(6, NULL, 0, 1, NN_SENDTO_ALL, 0);
            //printf("malloced %p\rt", cmd);
            //cmd_free(cmd);
            //printf("leed %p\rt", cmd);
            // make sure to check return value if que is full
            while(nn_router_add_cmd_req(rt0, cmd)){
                //assert(1 == 0);
                usleep(100);
            }

            //data = malloc(sizeof(*data));
            //nn_router_add_data_req(rt0, data);

            //usleep(10000);
            //sleep(1);
        }

        for(i=0; i < 300; i++){
            nn_node_free(n[i]);
        }
        //nn_node_free(n0);
        //nn_node_free(n1);
        nn_grp_free(g0);
        nn_router_free(rt0);
    }

    return 0;
}

/*
int io_cmd_req_cb(struct nn_router *rt, struct nn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct nn_io_conf *conf;

    assert(cmd);
    assert(cmd->conf);

    switch(cmd->conf->sendto_type){
        case NN_SENDTO_GRP:
            break;
        case NN_SENDTO_NODE:
            //send_cmd_to_node(n1, cmd);
            break;
        case NN_SENDTO_ALL:
            router_sendto_all(rt, cmd);
            //printf("freeing %p\rt", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}

int io_data_req_cb(struct nn_router *rt, struct nn_io_data *data)
{
    printf("ggg!!!!!!!!!!!!!!\rt");
    //printf("!!! yeah got it: %d\rt", data->id);
    //send_data_to_node(n1, data);
    //send_data_to_node(n0, data);
    //free(data);
    return 0;
}
*/


