
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "conn.h"
#include "router.h"

/* rt should be locked before calling this */
static int route_to_all(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r = 0;
    struct nn_conn *cn;
    void *iter;

    assert(rt);

    iter = NULL;
    while((cn=router_conn_iter(rt, &iter))){

        conn_lock(cn);
        //clone = cmd_clone(cmd);
        printf("!!! router_tx_cmd\n");
        if(conn_get_state(cn) != NN_LINK_STATE_DEAD){
           /* conn, can send */
            //while((cn=conn_router_tx_cmd(cn, cmd))){
            //    usleep(100);
            //}
            conn_unlock(cn);
            if(cn){
                goto err;
            }
        }else{
            L(LINFO, "Freeing dead conn");
            conn_unlock(cn);
            conn_free(cn);
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
    struct nn_conn *cn;
    struct nn_cmd *cmd;

    for(;;){
        printf("!! running route \n");

        router_lock(rt);

        // get iter for conns
        // unlock router
        iter = NULL;
        while((cn=router_conn_iter(rt, &iter))){

            conn_lock(cn);

            // router and one conn is locked but still all the nodes
            // can write to the other conns

            //cmd = conn_router_rx_cmd(rt, NULL);

            conn_unlock(cn);

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

