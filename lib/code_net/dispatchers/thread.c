
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"
#include "arch/thread.h"

#include "code_net/types.h"
#include "code_net/elem.h"

static void *thread_loop(void *arg)
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

    thread_detach(thread_self());

    for(;;){

        buf_check_timespec.tv_sec = 0;
        buf_check_timespec.tv_nsec = 10000000;
        cmd_check_timespec.tv_sec = 0;
        cmd_check_timespec.tv_nsec = 10000000;

        if((attr & CN_ATTR_NO_INPUT)){
            /* call user function */
            user_func(h, NULL, 0, pdata);

        }else{
            /* incoming data */
            buf = elem_read_in_buf(h, &buf_check_timespec);
            if(buf){
                /* call user function */
                user_func(h, buf, 1, pdata);
            }else{
                if(errno == ETIMEDOUT){
                    //printf("!! timedout xx\n");
                }
            }
            //sleep(1);
        }

        /* incoming commands */
        cmd_buf = elem_read_in_buf(h, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\n ");
            free(cmd_buf);
            break;
            // process and free
        }else{
            if(errno == ETIMEDOUT){
                //printf("!! timedout xx\n");
            }
        }

    }
end:
    printf("... thread exit\n");
    return NULL;
}

int dispatcher_thread(struct cn_elem *e) 
{
    thread_t tid;
    thread_create(&tid, NULL, thread_loop, e);
}
