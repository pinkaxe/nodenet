
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "node.h"
#include "pkt.h"
#include "conn.h"
#include "node_io.h"
#include "node_drivers/node_driver.h"
#include "node_drivers/thread.h"

static void *node_io_thread(void *arg);

int node_io_run(struct nn_node *n)
{
    int r = 0;
    thread_t tid;

    /* start the node thread that will handle coms from and to node */
    thread_create(&tid, NULL, node_io_thread, n);
    thread_detach(tid);

    return r;
}

/* free all node sides of all n->conn's and g->... */
static int node_conn_free(struct nn_node *n)
{
    int r = 0;
    struct node_conn_iter *iter;
    struct nn_conn *cn;

    iter = node_conn_iter_init(n);
    /* remove the conns to routers */
    while(!node_conn_iter_next(iter, &cn)){

        conn_lock(cn);

        node_unconn(n, cn);
        r = conn_free_node(cn);
        conn_unlock(cn);

        if(r == 1){
            conn_free(cn);
        }
    }
    node_conn_iter_free(iter);

    return r;

}

struct node_pdata {
    void *(*user_func)(struct nn_node *n, void *pdata);
    struct nn_node *n;
    void *pdata;
};

static void *start_user_thread(void *arg)
{
    struct node_pdata *info = arg;
    void *(*user_func)(struct nn_node *n, void *pdata) = info->user_func;
    struct nn_node *n = info->n;
    void *pdata = info->pdata;

    free(arg);

    return user_func(n, pdata);
}

/* rx input from conn, call user functions, tx output to conn  */
static void *node_io_thread(void *arg)
{
    struct nn_node *n = arg;
    struct timespec buf_check_timespec;
    struct timespec pkt_check_timespec;
    void *(*user_func)(struct nn_node *n, void *pdata);
    void *pdata;
    int attr;
    //struct node_driver_ops *ops;
    thread_t tid;
    int r;

    user_func = node_get_codep(n);
    pdata = node_get_pdatap(n);
    attr = node_get_attr(n);

    //ops = node_driver_get_ops(node_get_type(n));

    L(LNOTICE, "Node thread starting: %p", n);

    buf_check_timespec.tv_sec = 0;
    buf_check_timespec.tv_nsec = 10000000;
    pkt_check_timespec.tv_sec = 0;
    pkt_check_timespec.tv_nsec = 1000000;

    if(user_func){
        struct node_pdata *arg;
        PCHK(LWARN, arg, malloc(sizeof(*arg)));
        if(!arg){
            goto err;
        }
        arg->user_func = user_func;
        arg->n = n;
        arg->pdata = pdata;
        thread_create(&tid, NULL, start_user_thread, arg);
        thread_detach(tid);
    }

    for(;;){

        /* check state */
        switch(node_get_state(n)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                L(LNOTICE, "Node paused: %p", n);
                while(node_get_state(n) == NN_STATE_PAUSED){
                    sleep(1);
                    //node_cond_wait(n);
                }
                L(LNOTICE, "Node paused state exit: %p", n);
                break;
            case NN_STATE_SHUTDOWN:
                L(LNOTICE, "Node thread shutdown start: %p", n);
                node_conn_free(n);
                node_set_state(n, NN_STATE_FINISHED);
                //node_cond_broadcast(n);
                //node_unlock(n);
                L(LNOTICE, "Node thread shutdown completed");
                return NULL;
            case NN_STATE_FINISHED:
                L(LCRIT, "Node thread illegally in finished state");
                break;
        }

        ICHK(LDEBUG, r, node_tx_pkts(n));

        ICHK(LDEBUG, r, node_rx_pkts(n));

        usleep(100000);
    }

err:
    L(LNOTICE, "Node thread ended: %p", n);
    return NULL;
}
