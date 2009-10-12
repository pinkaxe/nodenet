
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
#if 0
struct nn_io_data_req {
    struct code_node *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

int nn_io_write_cmd(struct nn_node *n, enum nn_node_cmd cmd, void *pdata);

int nn_io_write_data(struct nn_node *n, struct io_buf_attr *attr, void *buf,
        jint len, void (*cleanup_cb)(void *buf, void *pdata));
#endif

static struct nn_node *e0, *e1, *e2;

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

//route_to_all
//route_to_node
//route_to_grp


/*
int io_cmd_req_cb(struct nn_router *rt, struct nn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct nn_io_conf *conf;

    assert(cmd);
    assert(cmd->conf);

    switch(cmd->conf->sendto_type){
        case nn_SENDTO_GRP:
            break;
        case nn_SENDTO_node:
            //send_cmd_to_node(e1, cmd);
            break;
        case nn_SENDTO_ALL:
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
    //send_data_to_node(e1, data);
    //send_data_to_node(e0, data);
    //free(data);
    return 0;
}
*/


int main(int argc, char *argv)
{
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

        e0 = nn_node_init(nn_TYPE_THREAD, nn_ATTR_NO_INPUT, input_node, NULL);
        ok(e0);

        //while(1){
        //sleep(5);
        nn_add_node_to_router(e0, rt0);
        nn_add_node_to_grp(e0, g0);
        //nn_rem_node_from_router(e0, rt0);


        e1 = nn_node_init(nn_TYPE_THREAD, 0, process_node, NULL);
        ok(e1);

        nn_add_node_to_router(e1, rt0);
        nn_add_node_to_grp(e1, g0);


        e2 = nn_node_init(nn_TYPE_LPROC, 0, process_lproc_node, NULL);
        ok(e2);

        //nn_add_node_to_router(e2, rt0);
        nn_add_node_to_grp(e2, g0);

        nn_node_run(e0);
        nn_node_run(e1);
        //nn_node_run(e2);
        //nn_node_run(e1);

        int i;
        for(i=0; i < 300; i++){
            n[i] = nn_node_init(nn_TYPE_THREAD, nn_ATTR_NO_INPUT, process_node, NULL);
            nn_add_node_to_router(n[i], rt0);
            nn_add_node_to_grp(n[i], g0);
            nn_node_run(n[i]);
        }

        for(i=0; i < 300; i++){
            //nn_node_run(n[i]);
        }

        //nn_router_set_cmd_cb(rt0, io_cmd_req_cb);
        //nn_router_set_data_cb(rt0, io_data_req_cb);
        nn_router_run(rt0);

        while(1){

            cmd = cmd_init(6, NULL, 0, 1, nn_SENDTO_ALL, 0);
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

        nn_node_free(e0);
        nn_node_free(e1);
        nn_grp_free(g0);
        nn_router_free(rt0);
    }

    return 0;
}
