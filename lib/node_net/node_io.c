
#include <stdio.h>
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


/* rx input from conn, call user functions, tx output to conn  */
static void *node_io_thread(void *arg)
{
    struct nn_node *n = arg;
    struct timespec buf_check_timespec;
    struct timespec pkt_check_timespec;
    void (*user_func)(struct nn_node *n, void *buf, int len, void *pdata);
    void *pdata;
    int attr;

    node_lock(n);

    user_func = node_get_codep(n);
    pdata = node_get_pdatap(n);
    attr = node_get_attr(n);

    L(LNOTICE, "Node thread starting: %p", n);

    node_unlock(n);

    buf_check_timespec.tv_sec = 0;
    buf_check_timespec.tv_nsec = 10000000;
    pkt_check_timespec.tv_sec = 0;
    pkt_check_timespec.tv_nsec = 1000000;

    for(;;){

        node_lock(n);

        /* check state */
        switch(node_get_state(n)){
            case NN_STATE_RUNNING:
                break;
            case NN_STATE_PAUSED:
                L(LNOTICE, "Node paused: %p", n);
                while(node_get_state(n) == NN_STATE_PAUSED){
                    node_cond_wait(n);
                }
                L(LNOTICE, "Node paused state exit: %p", n);
                break;
            case NN_STATE_SHUTDOWN:
                L(LNOTICE, "Node thread shutdown start: %p", n);
                node_conn_free(n);
                node_set_state(n, NN_STATE_FINISHED);
                node_cond_broadcast(n);
                node_unlock(n);
                L(LNOTICE, "Node thread shutdown completed");
                return NULL;
            case NN_STATE_FINISHED:
                L(LCRIT, "Node thread illegally in finished state");
                break;
        }

        node_unlock(n);

        /* rx/tx pkt/data */
        struct nn_pkt *pkt;

        NODE_CONN_ITER_PRE

        if(!conn_node_rx_pkt(cn, &pkt)){
            if(pkt){
                conn_unlock(cn);
                node_unlock(n);
                L(LNOTICE, "Node rx'd pkt, call driver");
                node_lock(n);
                conn_lock(cn);
                pkt_free(pkt);
            }
        }

        NODE_CONN_ITER_POST

        usleep(100000);

    }

    L(LNOTICE, "Node thread ended: %p", n);
    return NULL;
}
