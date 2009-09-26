
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


int input_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    //cn_io_write(e, 0, b, n, cleanup_cb);
    //L(LWARN, "loop\n");
    return 0;
}

int io_cmd_req_cb(struct cn_net *n, struct cn_io_cmd_req *req)
{
    printf("!!! got it\n");
}

int main(int argc, char *argv)
{
    struct cn_net *n0;
    struct cn_elem *e0, *e1, *e2;
    struct cn_elem *e[1024];
    struct cn_grp *g0;

    while(1){
        n0 = cn_net_init();
        ok(n0);

        g0 = cn_grp_init(1);
        ok(g0);

        e0 = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
        ok(e0);

        //while(1){
        //sleep(5);
        cn_add_elem_to_net(e0, n0);
        cn_add_elem_to_grp(e0, g0);
        //cn_rem_elem_from_net(e0, n0);


        e1 = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
        ok(e1);

        //while(1){
        //sleep(5);
        cn_add_elem_to_net(e1, n0);
        cn_add_elem_to_grp(e1, g0);
        //cn_rem_elem_from_net(e1, n0);

        cn_elem_run(e0);
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

        while(1) sleep(5);

        cn_elem_free(e0);
        cn_grp_free(g0);
        cn_net_free(n0);
    }

    return 0;
}
