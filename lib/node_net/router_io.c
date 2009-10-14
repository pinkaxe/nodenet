
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "link.h"
#include "router.h"

/* main io route threads */
static int route_to_router(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r = 0;
    struct nn_link_node_router *l;
    void *iter;
    struct nn_cmd *clone;

    assert(rt);

    router_lock(rt);

   // iter = NULL;
   // while((l=router_link_iter(rt, &iter))){

   //     link_lock(l);
   //     //clone = cmd_clone(cmd);
   //     printf("!!! router_tx_cmd\n");
   //     if(l->n){
   //        /* still linkected, can send */
   //        while((r=link_router_tx_cmd(l, cmd))){
   //            usleep(100);
   //        }
   //        if(r){
   //            link_unlock(l);
   //            goto err;
   //        }
   //     }

   //}

err:
    router_unlock(rt);
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
        printf("!! running route \n");

        router_lock(rt);
        //cmd = router_get_cmd(rt, NULL);
        cmd = link_router_rx_cmd(rt, NULL);
        router_unlock(rt);

        if(cmd){
            //rt->io_cmd_req_cb(rt, cmd);
            route_cmd(rt, cmd);
        }
        sleep(1);

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

