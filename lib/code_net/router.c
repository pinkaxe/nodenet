
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "net.h"
//#include "netpriv.h"

/* main io route threads */

static int route_to_net(struct cn_net *n, struct cn_cmd *cmd)
{
    int r = 0;
    struct cn_elem *e;
    void *iter;
    struct cn_cmd *clone;

    assert(n);

    iter = NULL;
    while((e=net_memb_iter(n, &iter))){
        clone = cmd_clone(cmd);
        while((r=elem_add_cmd(e, clone))){
            usleep(100);
        }
        if(r){
            goto err;
        }
   }

err:
    return r;
}


static int route_to_grp(struct cn_grp *g, struct cn_cmd *cmd)
{
}

static int route_to_elem(struct cn_net *n, struct cn_cmd *cmd)
{
}


/* decide who/where to route */
static int route_cmd(struct cn_net *n, struct cn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\n", cmd->id);
    //struct cn_io_conf *conf;

    assert(cmd);
    assert(cmd->conf);

    switch(cmd->conf->sendto_type){
        case CN_SENDTO_GRP:
            //route_to_grp(g, );
            break;
        case CN_SENDTO_ELEM:
            //send_cmd_to_elem(e1, cmd);
            break;
        case CN_SENDTO_ALL:
            route_to_net(n, cmd);
            //printf("freeing %p\n", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}


/* pick up cmds coming from net, and call router */
static void *route_cmd_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_net *n = arg;
    struct cn_cmd *cmd;

    for(;;){
        cmd = net_get_cmd(n, NULL);
        if(cmd){
            //n->io_cmd_req_cb(n, cmd);
            route_cmd(n, cmd);
        }

    }
}

/* pick up data coming from e and call router cb */
//static void *route_data_thread(void *arg)
//{
//    struct timespec ts = {0, 0};
//    struct cn_net *n = arg;
//    struct cn_io_data *data;
//
//    for(;;){
//        data = que_get(n->data_req, NULL);
//          cmd = net_get_cmd(n, NULL);
//        if(data){
//            n->io_data_req_cb(n, data);
//        }
//
//    }
//}


int io_route_run(struct cn_net *n)
{
    thread_t tid;
    thread_create(&tid, NULL, route_cmd_thread, n);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, n);
    //thread_detach(tid);

    return 0;
}

