
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "pkt.h"
#include "conn.h"
#include "router.h"


/* called when shutdown is received by router_io_thread */
static int router_conn_free(struct nn_router *rt)
{
    int r = 0;
    struct router_conn_iter *iter;
    struct nn_conn *cn;

    iter = router_conn_iter_init(rt);

    while(!router_conn_iter_next(iter, &cn)){

        conn_lock(cn);

        router_unconn(rt, cn);
        r = conn_free_router(cn);

        conn_unlock(cn);

        if(r == 1){
            conn_free(cn);
        }

    }

    router_conn_iter_free(iter);
    return 0;
}



static void _pause(struct nn_router *rt)
{
    L(LNOTICE, "Router paused: %p", rt);

    while(router_get_state(rt) == NN_STATE_PAUSED){
        router_cond_wait(rt);
    }

    L(LNOTICE, "Router unpaused %p", rt);
}

static void _shutdown(struct nn_router *rt)
{
    L(LNOTICE, "Route shutdown start: %p", rt);

    router_conn_free(rt);

    L(LNOTICE, "Router shutdown completed: %p", rt);
}


/* pick up pkts coming from router, and router to other node's */
static void *router_pkt_thread(void *arg)
{
    //struct timespec ts = {0, 0};
    struct nn_router *rt = arg;
    struct nn_pkt *pkt;

    L(LNOTICE, "Router thread starting");

    for(;;){

        router_lock(rt);

again:
        /* state changed ? */
        switch(router_get_state(rt)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                _pause(rt);
                goto again;
                break;
            case NN_STATE_SHUTDOWN:
                _shutdown(rt);
                router_set_state(rt, NN_STATE_FINISHED);
                router_cond_broadcast(rt);
                router_unlock(rt);
                thread_exit(NULL);
                break;
            case NN_STATE_FINISHED:
                L(LCRIT, "Illegal state");
                break;
        }

        router_unlock(rt);

        ROUTER_CONN_ITER_PRE

        /* rx pkt's and route */
        if(!conn_router_rx_pkt(cn, &pkt)){
            printf("ok\n");
            conn_unlock(cn);
            router_unlock(rt);

            // route data
            enum nn_pkt_pkt id = pkt_get_id(pkt);
            L(LNOTICE, "Router got pkt: %d\n", id);

            /* send */
            /* decide who to route to, and route */
            ROUTER_CONN_ITER_PRE

            while(conn_router_tx_pkt(rt, conn_get_node(cn), pkt)){
                usleep(1000);
            }

            ROUTER_CONN_ITER_POST

            pkt_free(pkt);

            router_lock(rt);
            conn_lock(cn);
        }

        /* rx data and route */
        // conn_router_rx_data()

        ROUTER_CONN_ITER_POST

        usleep(100000);
    }

}

int router_io_run(struct nn_router *rt)
{
    thread_t tid;
    thread_create(&tid, NULL, router_pkt_thread, rt);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, rt);
    //thread_detach(tid);

    return 0;
}

/* always check if the conn isn't dead before using it,
 * this function also frees the appropriate side of the
 * conn */
#if 0
static int if_conn_dead_free(struct nn_conn *cn)
{
    int r = 0;

    if(conn_get_state(cn) != NN_CONN_STATE_DEAD){
        /* we just free the router side but could read the buffers back or
         * something like that first */
        r = conn_free_router(cn);
        if(r == 1){
            conn_free(cn);
        }
    }
    return r;
}

#endif

/*

static int route_to_node_cb(struct nn_conn *cn, void *n_, void *pkt_)
{
    int r = 0;
    struct nn_node *n = n_;

    if(!if_conn_dead_free(cn) && conn_get_node(cn) == n){
         //r = conn_router_tx_pkt(cn, pkt);
    }
    return r;
}

static int route_to_node(struct nn_router *rt, struct nn_pkt *pkt)
{
    return router_conn_each(rt, route_to_node_cb, pkt);
}


static int route_to_all_cb(struct nn_conn *cn, void *pkt_)
{
    struct nn_pkt *pkt = pkt_;

    if(!if_conn_dead_free(cn)){
         //conn_router_tx_pkt(cn, pkt);
    }
}

static int route_to_all(struct nn_router *rt, struct nn_pkt *pkt)
{
    return router_conn_each(rt, route_to_all_cb, pkt);
}


static int route_to_grp(struct nn_grp *g, struct nn_pkt *pkt)
{
    //return grp_memb_each(g, route_to_grp_cb, pkt);
}
*/

#if 0
/* decide who/where to route */
static int route_pkt(struct nn_router *rt, struct nn_pkt *pkt)
{
    printf("route_pkt\n");
    //printf("!!! yeah got it: %d\rt", pkt->id);
    //struct nn_io_conf *conf;

    //assert(pkt);
    //assert(pkt->conf);

    //switch(pkt->conf->sendto_type){
    //    case NN_SENDTO_GRP:
    //        //route_to_grp(g, );
    //        break;
    //    case NN_SENDTO_NODE:
    //        //send_pkt_to_node(e1, pkt);
    //        break;
    //    case NN_SENDTO_ALL:
    //        route_to_all(rt, pkt);
    //        //printf("freeing %p\rt", pkt);
    //        //pkt_free(pkt);
    //        break;
    //    default:
    //        break;
    //}
    pkt_free(pkt);
    return 0;
}

#endif
#if 0
static int _router_tx_pkt()
{
    struct nn_pkt *pkt;
    pkt = pkt_init(33, NULL, 0, 0, sendto_type, int sendto_id);
    r = conn_router_tx_pkt(rt, &pkt);
    if(!r){
        printf("QQQ route pkt\n");
        //route_pkt(rt, pkt);
    }
    return r;
}
#endif
