
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <sys/socket.h>

#include "util/log.h"
#include "sys/thread.h"
#include "node_net/types.h"

/*
int dispatcher_lproc(struct nn_node *n) 
{
    thread_t tid;
    thread_create(&tid, NULL, thread_loop, n);
}
*/

static int fd[2];

void *lproc_middle(void *arg)
{
#if 0
    char buf[5];
    write(fd[0], "yes", 4);
    read(fd[0], buf, 3);
    fprintf(stderr, "!! read: %s\rt", buf);

    while(1){
        sleep(1);
        fprintf(stderr, "in middle\rt");
    }

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
        cmd_check_timespec.tv_nsec = 10000000;

        if((attr & NN_ATTR_NO_INPUT)){
            /* call user function */
            user_func(h, NULL, 0, pdata);

        }else{
            /* incoming data */
            buf = node_read_in_buf(h, &buf_check_timespec);
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

        /* incoming commands */
        cmd_buf = node_read_in_cmd(h, &cmd_check_timespec);
        if(cmd_buf){
            printf("!!! Got a cmd_buf\rt ");
            //exit(1);
            //free(cmd_buf);
            //break;
            // process and free
        }else{
            if(errno == ETIMEDOUT){
                //printf("!! timedout xxxx\rt");
            }
        }

    }
#endif
}

int lproc_loop()
{
    char buf[5];
    read(fd[1], buf, 4);
    printf("!! read0: %s\rt", buf);
    write(fd[1], "no", 3);
    while(1){
        sleep(1);
        printf("in process\rt");
    }
end:
    printf("... lproc exit\rt");
    return 0;

}


int dispatcher_lproc(struct nn_node *n)
{
    int r = 0;
    int pid;

    // create stream pipe
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    pid = fork();
    if(pid < 0){
        r = 1;
        goto err;
    }else if(pid > 0){
        // parent
        close(fd[1]);
        //printf("parent\rt");
        thread_t tid;
        thread_create(&tid, NULL, lproc_middle, n);
        thread_detach(tid);
    }else if(pid == 0){
        // child
        close(fd[0]);

        if(fd[1] != STDIN_FILENO)
            dup2(fd[1], STDIN_FILENO);

        if(fd[1] != STDOUT_FILENO)
            dup2(fd[1], STDOUT_FILENO);

        //printf("child\rt");
        lproc_loop();
        // or exec here
    }

err:
    return r;
}
