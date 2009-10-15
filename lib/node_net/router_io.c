
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "link.h"
#include "router.h"

/* rt should be locked before calling this */
static int route_to_all(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r = 0;
    struct nn_link *l;
    void *iter;

    assert(rt);

    iter = NULL;
    while((l=router_link_iter(rt, &iter))){

        link_lock(l);
        //clone = cmd_clone(cmd);
        printf("!!! router_tx_cmd\n");
        if(link_get_state(l) != NN_LINK_STATE_DEAD){
           /* link, can send */
            //while((l=link_router_tx_cmd(l, cmd))){
            //    usleep(100);
            //}
            link_unlock(l);
            if(l){
                goto err;
            }
        }else{
            L(LINFO, "Freeing dead link");
            link_unlock(l);
            link_free(l);
        }

   }

err:
    return r;
}


static int route_to_grp(struct nn_grp *g, struct nn_cmd *cmd)
{
}

static int route_to_node(struct nn_node *n, struct nn_cmd *cmd)
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
            route_to_all(rt, cmd);
            //printf("freeing %p\rt", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}


/* pick up cmds coming from router, and router to other node's */
static void *route_cmd_thread(void *arg)
{
    void *iter;
    struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_link *l;
    struct nn_cmd *cmd;

    for(;;){
        printf("!! running route \n");

        router_lock(rt);

        // get iter for links
        // unlock router
        iter = NULL;
        while((l=router_link_iter(rt, &iter))){

            link_lock(l);

            // router and one link is locked but still all the nodes
            // can write to the other links

            //cmd = link_router_rx_cmd(rt, NULL);

            link_unlock(l);

            if(cmd){
                //rt->io_cmd_req_cb(rt, cmd);
                printf("routing cmd\n");
                route_cmd(rt, cmd);
            }
        }

        router_unlock(rt);

        sleep(1);
        //return NULL;

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

