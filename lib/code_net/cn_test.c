
#include <unistd.h>
#include <assert.h>

#include "code_net/types.h"
#include "code_net/cn.h"

#define ok(x){ \
    assert(x); \
}

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


int input_elem(struct cn_elem *e, void *buf, int len, void *pdata)
{
    printf("xt!! \n");
    //cn_io_write(e, 0, b, n, cleanup_cb);
    return 0;
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

        printf("-- %p\n", n0);
        //while(1){
        //sleep(5);
        cn_add_elem_to_net(e0, n0);
        cn_add_elem_to_grp(e0, g0);
        //cn_rem_elem_from_net(e0, n0);


        e1 = cn_elem_init(CN_TYPE_THREAD, CN_ATTR_NO_INPUT, input_elem, NULL);
        ok(e1);

        printf("-- %p\n", n0);
        //while(1){
        //sleep(5);
        cn_add_elem_to_net(e1, n0);
        cn_add_elem_to_grp(e1, g0);
        //cn_rem_elem_from_net(e1, n0);

        //cn_elem_run(e0);
        //cn_elem_run(e1);

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

        while(1) sleep(5);

        cn_elem_free(e0);
        cn_grp_free(g0);
        cn_net_free(n0);
        printf("trough\n");
    }

    return 0;
}