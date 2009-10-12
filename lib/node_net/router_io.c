
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"

/* main io route threads */
static int route_to_router(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r = 0;
    struct nn_node *n;
    void *iter;
    struct nn_cmd *clone;

    assert(rt);

    iter = NULL;
    while((n=router_memb_iter(rt, &iter))){
        clone = cmd_clone(cmd);
        while((r=node_add_cmd(n, clone))){
            usleep(100);
        }
        if(r){
            goto err;
        }
   }

err:
    return r;
}


static int route_to_grp(struct nn_grp *g, struct nn_cmd *cmd)
{
}

static int route_to_node(struct nn_router *rt, struct nn_cmd *cmd)
{
}


/* decide who/where to route */
static int route_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct nn_io_conf *conf;

    assert(cmd);
    assert(cmd->conf);

    switch(cmd->conf->sendto_type){
        case NN_SENDTO_GRP:
            //route_to_grp(g, );
            break;
        case NN_SENDTO_NODE:
            //send_cmd_to_node(e1, cmd);
            break;
        case NN_SENDTO_ALL:
            route_to_router(rt, cmd);
            //printf("freeing %p\rt", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}


/* pick up cmds coming from router, and call router */
static void *route_cmd_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_cmd *cmd;

    for(;;){
        cmd = router_get_cmd(rt, NULL);
        if(cmd){
            //rt->io_cmd_req_cb(rt, cmd);
            route_cmd(rt, cmd);
        }

    }
}

/* pick up data coming from n and call router cb */
//static void *route_data_thread(void *arg)
//{
//    struct timespec ts = {0, 0};
//    struct nn_router *rt = arg;
//    struct nn_io_data *data;
//
//    for(;;){
//        data = que_get(rt->data_req, NULL);
//          cmd = router_get_cmd(rt, NULL);
//        if(data){
//            rt->io_data_req_cb(rt, data);
//        }
//
//    }
//}


int router_run(struct nn_router *rt)
{
    thread_t tid;
    thread_create(&tid, NULL, route_cmd_thread, rt);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, rt);
    //thread_detach(tid);

    return 0;
}

