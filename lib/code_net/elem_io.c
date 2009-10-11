
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "elem.h"
#include "elem_io.h"
#include "elem_types/elem_type.h"
#include "elem_types/thread.h"

//#include "code_net/elem_types/lproc.h"
//#include "code_net/elem_types/bin.h"

static void *elem_io_thread(void *arg);
static handle_int_cmd(struct cn_io_cmd *cmd);


/* start specific dispatcher */
int elem_io_run(struct cn_elem *e)
{
    int r = 0;
    thread_t tid;

    /* start the elem thread that will handle coms from and to elem */
    thread_create(&tid, NULL, elem_io_thread, e);
    thread_detach(tid);

    return r;
}


/* check element input/ouput and call user_func */
static void *elem_io_thread(void *arg)
{
    void *buf = NULL;
    void *cmd_buf = NULL;
    struct cn_elem *h = arg;
    struct timespec buf_check_timespec;
    struct timespec cmd_check_timespec;

    void (*user_func)(struct cn_elem *h, void *buf, int len, void *pdata) =
        elem_get_codep(h);
    void *pdata = elem_get_pdatap(h);
    int attr = elem_get_attr(h);

    for(;;){

        buf_check_timespec.tv_sec = 0;
        buf_check_timespec.tv_nsec = 10000000;
        cmd_check_timespec.tv_sec = 0;
        cmd_check_timespec.tv_nsec = 1000000;

        /* incoming commands */
        cmd_buf = elem_get_cmd(h, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\n");
            struct cn_io_cmd *cmd = cmd_buf;

            //int cmdid = cmd->cmd;
            handle_int_cmd(cmd);

            cmd_free(cmd);

            //switch(cmdid){
            //    case CN_ELEM_CMD_RUN:
            //        h->state = CN_ELEM_STATE_RUNNING;
            //        break;
            //    case CN_ELEM_CMD_PAUSE:
            //        h->state = CN_ELEM_STATE_PAUSE;
            //        break;
            //    case CN_ELEM_CMD_STOP:
            //        h->state = CN_ELEM_STATE_STOP0;
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
        //if(h->state == CN_ELEM_STATE_RUNNING){
            if((attr & CN_ATTR_NO_INPUT)){
                /* call user function */
                //user_func(h, NULL, 0, pdata);
                //h->ops->elem_buf_exe(h, NULL, 0, pdata);
            }else{
                /* incoming data */
                buf = elem_get_buf(h, &buf_check_timespec);
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
    printf("elem_io exit\rt");
    return NULL;
}


static handle_int_cmd(struct cn_io_cmd *cmd)
{
};

