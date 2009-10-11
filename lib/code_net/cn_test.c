
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util/log.h"

#include "code_net/types.h"
#include "code_net/cmd.h"
#include "code_net/cn.h"

#define ok(x){ \
    assert(x); \
}

#define GRP0 0
#if 0
struct cn_io_data_req {
    struct code_elem *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

int cn_io_write_cmd(struct cn_elem *e, enum cn_elem_cmd cmd, void *pdata);

int cn_io_write_data(struct cn_elem *e, struct io_buf_attr *attr, void *buf,
        jint len, void (*cleanup_cb)(void *buf, void *pdata));
#endif

static struct cn_elem *e0, *e1, *e2;

int input_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    //j
    //cn_io_write(e, 0, b, rt, cleanup_cb);
    //L(LWARN, "loop\rt");
    return 0;
}

int process_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    printf("!! process\n");
    //e->add_output_data();
    return 0;
}

int process_lproc_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    printf("!! process lproc_\n");
    return 0;
}

//route_to_all
//route_to_elem
//route_to_grp


/*
int io_cmd_req_cb(struct cn_router *rt, struct cn_cmd *cmd)
{
    //printf("!!! yeah got it: %d\rt", cmd->id);
    //struct cn_io_conf *conf;

    assert(cmd);
    assert(cmd->conf);

    switch(cmd->conf->sendto_type){
        case CN_SENDTO_GRP:
            break;
        case CN_SENDTO_ELEM:
            //send_cmd_to_elem(e1, cmd);
            break;
        case CN_SENDTO_ALL:
            router_sendto_all(rt, cmd);
            //printf("freeing %p\rt", cmd);
            cmd_free(cmd);
            break;
        default:
            break;
    }
    //free(cmd);
    return 0;
}

int io_data_req_cb(struct cn_router *rt, struct cn_io_data *data)
{
    printf("ggg!!!!!!!!!!!!!!\rt");
    //printf("!!! yeah got it: %d\rt", data->id);
    //send_data_to_elem(e1, data);
    //send_data_to_elem(e0, data);
    //free(data);
    return 0;
}
*/


int main(int argc, char *argv)
{
    struct cn_router *rt0;
    struct cn_elem *e[1024];
    struct cn_grp *g0;
    struct cn_cmd *cmd;
    struct cn_io_data *data;
    struct cn_io_conf *conf;

    while(1){
        rt0 = cn_router_init();
        ok(rt0);

        g0 = cn_grp_init(GRP0);
        ok(g0);

        e0 = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
        ok(e0);

        //while(1){
        //sleep(5);
        cn_add_elem_to_router(e0, rt0);
        cn_add_elem_to_grp(e0, g0);
        //cn_rem_elem_from_router(e0, rt0);


        e1 = cn_elem_init(CN_TYPE_THREAD, 0, process_elem, NULL);
        ok(e1);

        cn_add_elem_to_router(e1, rt0);
        cn_add_elem_to_grp(e1, g0);


        e2 = cn_elem_init(CN_TYPE_LPROC, 0, process_lproc_elem, NULL);
        ok(e2);

        //cn_add_elem_to_router(e2, rt0);
        cn_add_elem_to_grp(e2, g0);

        cn_elem_run(e0);
        cn_elem_run(e1);
        //cn_elem_run(e2);
        //cn_elem_run(e1);

        int i;
        for(i=0; i < 300; i++){
            e[i] = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, process_elem, NULL);
            cn_add_elem_to_router(e[i], rt0);
            cn_add_elem_to_grp(e[i], g0);
            cn_elem_run(e[i]);
        }

        for(i=0; i < 300; i++){
            //cn_elem_run(e[i]);
        }

        //cn_router_set_cmd_cb(rt0, io_cmd_req_cb);
        //cn_router_set_data_cb(rt0, io_data_req_cb);
        cn_router_run(rt0);

        while(1){

            cmd = cmd_init(6, NULL, 0, 1, CN_SENDTO_ALL, 0);
            //printf("malloced %p\rt", cmd);
            //cmd_free(cmd);
            //printf("leed %p\rt", cmd);
            // make sure to check return value if que is full
            while(cn_router_add_cmd_req(rt0, cmd)){
                //assert(1 == 0);
                usleep(100);
            }

            //data = malloc(sizeof(*data));
            //cn_router_add_data_req(rt0, data);

            //usleep(10000);
            //sleep(1);
        }

        cn_elem_free(e0);
        cn_elem_free(e1);
        cn_grp_free(g0);
        cn_router_free(rt0);
    }

    return 0;
}
