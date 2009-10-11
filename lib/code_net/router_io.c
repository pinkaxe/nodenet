
#include <stdio.h>
#include <assert.h>

#include "sys/thread.h"
#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"

/* main io route threads */
static int route_to_router(struct cn_router *rt, struct cn_cmd *cmd)
{
    int r = 0;
    struct cn_elem *e;
    void *iter;
    struct cn_cmd *clone;

    assert(rt);

    iter = NULL;
    while((e=router_memb_iter(rt, &iter))){
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

static int route_to_elem(struct cn_router *rt, struct cn_cmd *cmd)
{
}


/* decide who/where to route */
static int route_cmd(struct cn_router *rt, struct cn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\rt", cmd->id);
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
            route_to_router(rt, cmd);
            //printf("freeing %p\rt", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}


/* pick up cmds coming from router, and call router */
static void *route_cmd_thread(void *arg)
{
    struct timespec ts = {0, 0};
    struct cn_router *rt = arg;
    struct cn_cmd *cmd;

    for(;;){
        cmd = router_get_cmd(rt, NULL);
        if(cmd){
            //rt->io_cmd_req_cb(rt, cmd);
            route_cmd(rt, cmd);
        }

    }
}

/* pick up data coming from e and call router cb */
//static void *route_data_thread(void *arg)
//{
//    struct timespec ts = {0, 0};
//    struct cn_router *rt = arg;
//    struct cn_io_data *data;
//
//    for(;;){
//        data = que_get(rt->data_req, NULL);
//          cmd = router_get_cmd(rt, NULL);
//        if(data){
//            rt->io_data_req_cb(rt, data);
//        }
//
//    }
//}


int router_run(struct cn_router *rt)
{
    thread_t tid;
    thread_create(&tid, NULL, route_cmd_thread, rt);
    thread_detach(tid);

    //thread_create(&tid, NULL, route_data_thread, rt);
    //thread_detach(tid);

    return 0;
}

