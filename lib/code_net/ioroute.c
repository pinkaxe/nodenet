
#include <stdio.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/que.h"

#include "types.h"
#include "io.h"
#include "netpriv.h"

/* main io route threads */

/* pick up cmds coming from e, and call router */
static void *cmd_in_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_net *n = arg;
    struct cn_io_cmd *cmd;

    for(;;){
        cmd = que_get(n->cmd_req, NULL);
        if(cmd){
            n->io_cmd_req_cb(n, cmd);
        }

    }
}

/* pick up data coming from e and call router cb */
static void *data_in_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_net *n = arg;
    struct cn_io_data *data;

    for(;;){
        data = que_get(n->data_req, NULL);
        if(data){
            n->io_data_req_cb(n, data);
        }

    }
}


int ioroute_run(struct cn_net *n)
{
    thread_t tid;
    thread_create(&tid, NULL, cmd_in_thread, n);
    thread_detach(tid);

    thread_create(&tid, NULL, data_in_thread, n);
    thread_detach(tid);

    return 0;
}

