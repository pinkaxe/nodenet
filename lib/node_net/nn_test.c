
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

static struct nn_node *n[0], *n1, *n2;

int input_node(struct nn_node *n, void *buf, int len, void *pdata)
{
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

int output_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    int i;

    //n->tx_cmd(n, );
    //nn_node_tx_cmd(n, buf, len, destiny);
    //nn_node_tx(n, );
}

#if 0
//int adder_node(struct nn_node *n, void *buf, int len, void *pdata)
//{
//    uint8_t *val = buf;
//
//    malloc
//}


int xmain(int argc, char *argv)
{
    int i;
    struct nn_router *rt[1024];
    struct nn_node *n[1024];
    struct nn_grp *g[1024];
    struct nn_conn *cn[1024];
    struct nn_cmd *cmd;

    rt[0] = nn_router_init();
    ok(rt[0]);

    for(i=0; i < 3; i++){
        n[0] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                adder_node, NULL);
    }

    return 0;
}
#endif

int main(int argc, char *argv)
{
    int i;
    struct nn_router *rt[1024];
    struct nn_node *n[1024];
    struct nn_grp *g[1024];
    struct nn_conn *cn[1024];
    struct nn_cmd *cmd;
    //struct nn_io_data *data;
    //struct nn_io_conf *conf;

    while(1){

        rt[0] = nn_router_init();
        ok(rt[0]);

        for(i=0; i < 3; i++){
            g[i] = nn_grp_init(i);
            ok(g[i]);
        }

        /* create input nodes */
        for(i=0; i < 100; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT, input_node, NULL);
            nn_conn(n[i], rt[0]);
            //nn_node_join_grp(n[i], rt[0]);

            //conn_conn(n[i], rt[0]);
            //nn_join_grp(n[i], g[1]);
            ok(n[i]);
        }

        /* create process nodes */
        for(;i < 200; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, 0, process_node, NULL);
            nn_conn(n[i], rt[0]);
            //nn_join_grp(n[i], g[1]);
        }

        /* create output nodes */
        for(;i < 300; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, 0, output_node, NULL);
            nn_conn(n[i], rt[0]);
            //nn_join_grp(n[i], g[2]);
        }

        /* create output nodes */
        /*
        for(;i < 300; i++){
            ok = nn_node_isok(n[i]);
            if(!ok){
                printf("!! bad shit, n[i]\n");
            }
        }
        */

       // nn_router_run(rt[0]);

       // for(i = 0; i < 300; i++){
       //     nn_node_run(n[i]);
       // }

        /*
        //nn_router_set_cmd_cb(rt[0], io_cmd_req_cb);
        //nn_router_set_data_cb(rt[0], io_data_req_cb);

        //for(;;){
        for(i = 0; i < 1; i++){

            cmd = cmd_init(6, NULL, 0, 1, NN_SENDTO_ALL, 0);
            //printf("malloced %p\rt", cmd);
            //cmd_free(cmd);
            //printf("leed %p\rt", cmd);
            // make sure to check return value if que is full
            while(nn_router_tx_cmd(rt[0], cmd)){
                //assert(1 == 0);
                //printf("!! bla\n");
                usleep(100);
            }

            //data = malloc(sizeof(*data));
            //nn_router_tx_data(rt[0], data);

            //usleep(10000);
        }
        sleep(5);
        */

        for(i=0; i < 300; i++){
            /* unconn not needed but ok */
            nn_unconn(n[i], rt[0]);
            nn_node_free(n[i]);
        }

        nn_grp_free(g[0]);
        nn_grp_free(g[1]);
        nn_grp_free(g[2]);

        nn_router_free(rt[0]);

        nn_wait();
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
    //send_data_to_node(n[0], data);
    //free(data);
    return 0;
}
*/


