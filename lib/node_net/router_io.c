
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



static int route_to_node_cb(struct nn_conn *cn, void *n_, void *icmd_)
{
    int r = 0;
    struct nn_node *n = n_;
    struct nn_icmd *icmd = icmd_;

    if(!if_conn_dead_free(cn) && conn_get_node(cn) == n){
         //r = conn_router_tx_cmd(cn, icmd);
    }
    return r;
}

static int route_to_node(struct nn_router *rt, struct nn_icmd *icmd)
{
    return router_conn_each(rt, route_to_node_cb, icmd);
}


static int route_to_all_cb(struct nn_conn *cn, void *icmd_)
{
    struct nn_icmd *icmd = icmd_;

    if(!if_conn_dead_free(cn)){
         //conn_router_tx_cmd(cn, icmd);
    }
}

static int route_to_all(struct nn_router *rt, struct nn_icmd *icmd)
{
    return router_conn_each(rt, route_to_all_cb, icmd);
}


static int route_to_grp(struct nn_grp *g, struct nn_icmd *icmd)
{
    //return grp_memb_each(g, route_to_grp_cb, icmd);
}



/* decide who/where to route */
static int route_cmd(struct nn_router *rt, struct nn_icmd *icmd)
{
    printf("route_cmd\n");
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct nn_io_conf *conf;

    //assert(icmd);
    //assert(icmd->conf);

    //switch(icmd->conf->sendto_type){
    //    case NN_SENDTO_GRP:
    //        //route_to_grp(g, );
    //        break;
    //    case NN_SENDTO_NODE:
    //        //send_cmd_to_node(e1, cmd);
    //        break;
    //    case NN_SENDTO_ALL:
    //        route_to_all(rt, icmd);
    //        //printf("freeing %p\rt", cmd);
    //        //cmd_free(cmd);
    //        break;
    //    default:
    //        break;
    //}
    icmd_free(icmd);
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
static int _router_tx_icmd()
{
    struct nn_icmd *icmd;
    icmd = icmd_init(33, NULL, 0, 0, sendto_type, int sendto_id);
    r = conn_router_tx_icmd(rt, &icmd);
    if(!r){
        printf("QQQ route cmd\n");
        //route_cmd(rt, icmd);
    }
    return r;
}
#endif

/* pick up cmds coming from router, and router to other node's */
static void *router_icmd_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_conn *cn;
    int r;

    L(LNOTICE, "Router thread starting");

    for(;;){
        router_lock(rt);

        switch(router_get_state(rt)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                L(LNOTICE, "Router paused: %p", rt);
                while(router_get_state(rt) == NN_STATE_PAUSED){
                    router_cond_wait(rt);
                }
                L(LNOTICE, "Router paused state exit: %p", rt);
                break;
            case NN_STATE_SHUTDOWN:
                L(LNOTICE, "Router shutdown start: %p", rt);
                router_unlock(rt);
                router_conn_free(rt);
                router_free(rt);
                busy_freeing_no--;
                L(LNOTICE, "Router shutdown completed");
                return NULL;
        }

        router_unlock(rt);

        /* check cmd_in, data_in */

        //while(router_get_state(rt) == NN_STATE_PAUSED){
        //    router_unlock(rt);
        //    usleep(500000);
        //    router_lock(rt);
        //}

        //if(router_get_state(rt) == NN_STATE_SHUTDOWN){
        //    router_unlock(rt);
        //    L(LNOTICE, "Router shutdown start: %p", rt);
        //    router_conn_free(rt);
        //    router_free(rt);
        //    busy_freeing_no--;
        //    L(LNOTICE, "Router shutdown completed: %p", rt);
        //    return NULL;
        //}

        sleep(1);
    }
}

int router_io_run(struct nn_router *rt)
{
    thread_t tid;
    thread_create(&tid, NULL, router_icmd_thread, rt);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, rt);
    //thread_detach(tid);

    return 0;
}
