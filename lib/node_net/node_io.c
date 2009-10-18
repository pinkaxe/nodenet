
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "node.h"
#include "conn.h"
#include "node_io.h"
#include "node_drivers/node_driver.h"
#include "node_drivers/thread.h"

//#include "node_net/node_drivers/lproc.n"
//#include "node_net/node_drivers/bin.n"

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


   // /* remove pointers from groups */
   // iter = NULL;
   // while((g=node_grps_iter(n, &iter))){
   //     grp_rem_node(g, n);
   // }



    return r;

}

extern int busy_freeing_no;

/* rx input from conn, call user functions, tx output to conn  */
static void *node_io_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    struct nn_node *n = arg;
    struct timespec buf_check_timespec;
    struct timespec icmd_check_timespec;
    void (*user_func)(struct nn_node *n, void *buf, int len, void *pdata);
    void *pdata;
    int attr;

    node_lock(n);

    user_func = node_get_codep(n);
    pdata = node_get_pdatap(n);
    attr = node_get_attr(n);

    node_unlock(n);

    buf_check_timespec.tv_sec = 0;
    buf_check_timespec.tv_nsec = 10000000;
    icmd_check_timespec.tv_sec = 0;
    icmd_check_timespec.tv_nsec = 1000000;

    L(LNOTICE, "Node thread starting: %p", n);

    for(;;){
        sleep(1);

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
                busy_freeing_no--;
                node_set_state(n, NN_STATE_FINISHED);
                node_cond_broadcast(n);
                node_unlock(n);
                L(LNOTICE, "Node thread shutdown completed");
                return NULL;
        }

        /* rx/tx cmd/data */
        struct nn_icmd *icmd;
        //icmd = conn_node_rx_icmd(n, &icmd_check_timespec);
        node_unlock(n);
        if(icmd){
            // call driver function
            //n->ops->rx_icmd(n, icmd);
        }

//        node_lock(n);
//        //cmd = conn_node_rx_cmd(n, &cmd_check_timespec);
//        node_unlock(n);
//        if(cmd){
//            // call driver function
//        }
//
//        node_lock(n);
//        //data = conn_node_rx_data(n, &data_check_timespec);
//        node_unlock(n);
//        if(data){
//            // call driver function to send to node endpoint
//        }

        node_lock(n);
        // check for output for node driver
        node_unlock(n);

    }

end:
    L(LNOTICE, "Node thread ended: %p", n);
    return NULL;
}



        /* incoming commands */
        // node_lock(n);
        //cmd_buf = node_get_cmd(n, &cmd_check_timespec);
        //cmd_buf = conn_node_rx_cmd(n, &cmd_check_timespec);
        // node_unlock(n);

       // if(cmd_buf){
       //     printf("!!! Got a cmd_buf\n");
       //     struct nn_cmd *cmd = cmd_buf;

       //     //int cmdid = cmd->cmd;
       //     handle_int_cmd(cmd);

       //     cmd_free(cmd);
       //     goto end;

       //     //switch(cmdid){
       //     //    case NN_NODE_STATE_RUN:
       //     //        n->state = nn_node_STATE_RUNNING;
       //     //        break;
       //     //    case NN_NODE_STATE_PAUSE:
       //     //        n->state = nn_node_STATE_PAUSE;
       //     //        break;
       //     //    case NN_CMD_CMD_STOP:
       //     //        n->state = nn_node_STATE_STOP0;
       //     //        goto end;

       //     //printf("!!! freed cmd_buf\rt ");
       //     //exit(1);
       //     //free(cmd_buf);
       //     //break;
       //     // process and free
       // }else{
       //     if(errno == ETIMEDOUT){
       //         //printf("!! timedout xxxx\rt");
       //     }
       // }

#if 0
        /* incoming data */
        //if(n->state == nn_node_STATE_RUNNING){
            if((attr & NN_NODE_ATTR_NO_INPUT)){
                /* call user function */
                //user_func(n, NULL, 0, pdata);
                //n->ops->node_buf_exe(n, NULL, 0, pdata);
            }else{
                /* incoming data */
                buf = node_get_buf(n, &buf_check_timespec);
                if(buf){
                    /* call user function */
                    user_func(n, buf, 1, pdata);
                }else{
                    if(errno == ETIMEDOUT){
                        //printf("!! timedout xx\rt");
                    }
                }
                //sleep(1);
            }
        //}
#endif


