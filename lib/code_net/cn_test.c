
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "util/log.h"

#include "code_net/types.h"
#include "code_net/io.h"
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
        int len, void (*cleanup_cb)(void *buf, void *pdata));
#endif

static struct cn_elem *e0, *e1, *e2;

int input_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    //j
    //cn_io_write(e, 0, b, n, cleanup_cb);
    //L(LWARN, "loop\n");
    return 0;
}

int process_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    printf("!! process\n");
    return 0;
}

int process_lproc_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    printf("!! process lproc_\n");
    return 0;
}

int io_cmd_req_cb(struct cn_net *n, struct cn_io_cmd *cmd)
{
    //printf("!!! yeah got it: %d\n", cmd->id);
    send_cmd_to_elem(e1, cmd);
    send_cmd_to_elem(e0, cmd);
    //free(cmd);
    return 0;
}

int main(int argc, char *argv)
{
    struct cn_net *n0;
    struct cn_elem *e[1024];
    struct cn_grp *g0;
    struct cn_io_cmd *cmd;
    struct cn_io_conf *conf;

    while(1){
        n0 = cn_net_init();
        ok(n0);

        g0 = cn_grp_init(GRP0);
        ok(g0);

        e0 = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
        ok(e0);

        //while(1){
        //sleep(5);
        cn_add_elem_to_net(e0, n0);
        cn_add_elem_to_grp(e0, g0);
        //cn_rem_elem_from_net(e0, n0);


        e1 = cn_elem_init(CN_TYPE_THREAD, 0, process_elem, NULL);
        ok(e1);

        cn_add_elem_to_net(e1, n0);
        cn_add_elem_to_grp(e1, g0);


        e2 = cn_elem_init(CN_TYPE_LPROC, 0, process_lproc_elem, NULL);
        ok(e2);

        cn_add_elem_to_net(e2, n0);
        cn_add_elem_to_grp(e2, g0);

        //cn_elem_run(e0);
        //cn_elem_run(e1);
        cn_elem_run(e2);
        //cn_elem_run(e1);

        /*
        int i;
        for(i=0; i < 300; i++){
            e[i] = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
            cn_add_elem_to_net(e[i], n0);
            cn_add_elem_to_grp(e[i], g0);
            cn_elem_run(e[i]);
        }

        for(i=0; i < 300; i++){
            //cn_elem_run(e[i]);
        }
        */

        cn_net_set_cmd_cb(n0, io_cmd_req_cb);
        cn_net_run(n0);

        while(1){
           // cmd = malloc(sizeof(*cmd));
           // cmd->id = 66;
           // cn_net_add_cmd_req(n0, cmd);
            usleep(1000);
        }

        cn_elem_free(e0);
        cn_elem_free(e1);
        cn_grp_free(g0);
        cn_net_free(n0);
    }

    return 0;
}
