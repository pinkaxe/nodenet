
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
static handle_int_cmd(struct nn_cmd *cmd);


int node_io_run(struct nn_node *n)
{
    int r = 0;
    thread_t tid;

    /* start the node thread that will handle coms from and to node */
    thread_create(&tid, NULL, node_io_thread, n);
    thread_detach(tid);

    return r;
}


/* rx input from conn, call user functions, tx output to conn  */
static void *node_io_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    struct nn_node *n = arg;
    struct timespec buf_check_timespec;
    struct timespec cmd_check_timespec;
    void (*user_func)(struct nn_node *n, void *buf, int len, void *pdata);
    void *pdata;
    int attr;

    node_lock(n);

    user_func = node_get_codep(n);
    pdata = node_get_pdatap(n);
    attr = node_get_attr(n);

    node_unlock(n);

    for(;;){

        buf_check_timespec.tv_sec = 0;
        buf_check_timespec.tv_nsec = 10000000;
        cmd_check_timespec.tv_sec = 0;
        cmd_check_timespec.tv_nsec = 1000000;
        sleep(1);
        printf("node_io_loop\n");

        /* incoming commands */
       // node_lock(n);
       // //cmd_buf = node_get_cmd(n, &cmd_check_timespec);
       // cmd_buf = conn_node_rx_cmd(n, &cmd_check_timespec);
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


    }
end:
    printf("node_io exit\rt");
    return NULL;
}


static handle_int_cmd(struct nn_cmd *cmd)
{
};

