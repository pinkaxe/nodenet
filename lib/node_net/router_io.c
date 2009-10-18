
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "conn.h"
#include "router.h"

extern int busy_freeing_no;

/* always check if the conn isn't dead before using it,
 * this function also frees the appropriate side of the
 * conn */
static int if_conn_dead_free(struct nn_conn *cn)
{
    int r = 0;

    if(conn_get_state(cn) != NN_CONN_STATE_DEAD){
        /* we just free the router side but could read the buffers back or
         * something like that first */
        conn_free_router(cn);
        r = 1;
    }
    return r;
}



static int route_to_node_cb(struct nn_conn *cn, void *n_, void *cmd_)
{
    int r = 0;
    struct nn_node *n = n_;
    struct nn_cmd *cmd = cmd_;

    if(!if_conn_dead_free(cn) && conn_get_node(cn) == n){
         //r = conn_router_tx_cmd(cn, cmd);
    }
    return r;
}

static int route_to_node(struct nn_router *rt, struct nn_cmd *cmd)
{
    return router_conn_each(rt, route_to_node_cb, cmd);
}


static int route_to_all_cb(struct nn_conn *cn, void *cmd_)
{
    struct nn_cmd *cmd = cmd_;

    if(!if_conn_dead_free(cn)){
         //conn_router_tx_cmd(cn, cmd);
    }
}

static int route_to_all(struct nn_router *rt, struct nn_cmd *cmd)
{
    return router_conn_each(rt, route_to_all_cb, cmd);
}


static int route_to_grp(struct nn_grp *g, struct nn_cmd *cmd)
{
    //return grp_memb_each(g, route_to_grp_cb, cmd);
}



/* decide who/where to route */
static int route_cmd(struct nn_router *rt, struct nn_cmd *cmd)
{
    printf("route_cmd\n");
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct nn_io_conf *conf;

    //assert(cmd);
    //assert(cmd->conf);

    //switch(cmd->conf->sendto_type){
    //    case NN_SENDTO_GRP:
    //        //route_to_grp(g, );
    //        break;
    //    case NN_SENDTO_NODE:
    //        //send_cmd_to_node(e1, cmd);
    //        break;
    //    case NN_SENDTO_ALL:
    //        route_to_all(rt, cmd);
    //        //printf("freeing %p\rt", cmd);
    //        //cmd_free(cmd);
    //        break;
    //    default:
    //        break;
    //}
    cmd_free(cmd);
    return 0;
}



/* called when shutdown is received by router_io_thread */
static int router_conn_free(struct nn_router *rt)
{
    struct router_conn_iter *iter;
    struct nn_conn *cn;

    router_lock(rt);

    iter = router_conn_iter_init(rt);

    while(!router_conn_iter_next(iter, &cn)){

        conn_lock(cn);

        router_unconn(rt, cn);

        conn_unlock(cn);
        router_unlock(rt);

        conn_free_router(cn);

        router_lock(rt);
    }

    router_conn_iter_free(iter);

    router_unlock(rt);


    return 0;
}


#if 0
static int _router_tx_cmd()
{
    struct nn_cmd *cmd;
    cmd = cmd_init(33, NULL, 0, 0, sendto_type, int sendto_id);
    r = conn_router_tx_cmd(rt, &cmd);
    if(!r){
        printf("QQQ route cmd\n");
        //route_cmd(rt, cmd);
    }
    return r;
}
#endif

static void pause(struct nn_router *rt)
{
    L(LNOTICE, "Router paused: %p", rt);

    while(router_get_state(rt) == NN_STATE_PAUSED){
        router_cond_wait(rt);
    }

    L(LNOTICE, "Router unpaused %p", rt);
}

static void shutdown(struct nn_router *rt)
{
    L(LNOTICE, "Route shutdown start: %p", rt);

    router_conn_free(rt);
    router_free(rt);
    busy_freeing_no--;

    L(LNOTICE, "Router shutdown completed: %p", rt);
}


/* pick up cmds coming from router, and router to other node's */
static void *router_cmd_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_conn *cn;
    struct nn_cmd *cmd;
    int r;

    L(LNOTICE, "Router thread starting");

    for(;;){

        router_lock(rt);

        /* state changed ? */
        switch(router_get_state(rt)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                pause(rt);
                break;
            case NN_STATE_SHUTDOWN:
                router_unlock(rt);
                shutdown(rt);
                thread_exit(NULL);
                break;
        }

        router_unlock(rt);

        ROUTER_CONN_ITER_PRE

        /* rx cmd's and route */
        if(!conn_router_rx_cmd(cn, &cmd)){
            conn_unlock(cn);
            router_unlock(rt);

            // route data
            enum nn_cmd_cmd id = cmd_get_id(cmd);
            printf("!! router got cmd: %d\n", id);
            cmd_free(cmd);

            router_lock(rt);
            conn_lock(cn);
        }

        /* rx data and route */
        // conn_router_rx_data()

        ROUTER_CONN_ITER_POST

        usleep(10000);
    }

}

int router_io_run(struct nn_router *rt)
{
    thread_t tid;
    thread_create(&tid, NULL, router_cmd_thread, rt);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, rt);
    //thread_detach(tid);

    return 0;
}
