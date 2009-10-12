
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "node.h"
#include "node_io.h"
#include "node_types/node_type.h"
#include "node_types/thread.h"

//#include "node_net/node_types/lproc.h"
//#include "node_net/node_types/bin.h"

static void *node_io_thread(void *arg);
static handle_int_cmd(struct nn_cmd *cmd);


/* start specific dispatcher */
int node_io_run(struct nn_node *n)
{
    int r = 0;
    thread_t tid;

    /* start the node thread that will handle coms from and to node */
    thread_create(&tid, NULL, node_io_thread, n);
    thread_detach(tid);

    return r;
}


/* check nodeent input/ouput and call user_func */
static void *node_io_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    struct nn_node *h = arg;
    struct timespec buf_check_timespec;
    struct timespec cmd_check_timespec;

    void (*user_func)(struct nn_node *h, void *buf, int len, void *pdata) =
        node_get_codep(h);
    void *pdata = node_get_pdatap(h);
    int attr = node_get_attr(h);

    for(;;){

        buf_check_timespec.tv_sec = 0;
        buf_check_timespec.tv_nsec = 10000000;
        cmd_check_timespec.tv_sec = 0;
        cmd_check_timespec.tv_nsec = 1000000;

        /* incoming commands */
        cmd_buf = node_get_cmd(h, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\n");
            struct nn_cmd *cmd = cmd_buf;

            //int cmdid = cmd->cmd;
            handle_int_cmd(cmd);

            cmd_free(cmd);

            //switch(cmdid){
            //    case NN_NODE_CMD_RUN:
            //        h->state = nn_node_STATE_RUNNING;
            //        break;
            //    case NN_NODE_CMD_PAUSE:
            //        h->state = nn_node_STATE_PAUSE;
            //        break;
            //    case NN_NODE_CMD_STOP:
            //        h->state = nn_node_STATE_STOP0;
            //        goto end;

            //printf("!!! freed cmd_buf\rt ");
            //exit(1);
            //free(cmd_buf);
            //break;
            // process and free
        }else{
            if(errno == ETIMEDOUT){
                //printf("!! timedout xxxx\rt");
            }
        }

        /* incoming data */
        //if(h->state == nn_node_STATE_RUNNING){
            if((attr & NN_ATTR_NO_INPUT)){
                /* call user function */
                //user_func(h, NULL, 0, pdata);
                //h->ops->node_buf_exe(h, NULL, 0, pdata);
            }else{
                /* incoming data */
                buf = node_get_buf(h, &buf_check_timespec);
                if(buf){
                    /* call user function */
                    user_func(h, buf, 1, pdata);
                }else{
                    if(errno == ETIMEDOUT){
                        //printf("!! timedout xx\rt");
                    }
                }
                //sleep(1);
            }
        //}


    }
end:
    printf("node_io exit\rt");
    return NULL;
}


static handle_int_cmd(struct nn_cmd *cmd)
{
};

