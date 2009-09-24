
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include "code_net/code.h"
#include "code_net/code_grp.h"

#include "util/dpool.h"

/*
void sending_to_no_cb(void *buf, int no)
{
   struct dpool_buf *b = buf;
   // // FIXME: do through function
   b->ref_cnt = no;
}
*/

void cleanup_cb(void *buf, void *pdata)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *b = pdata;

    dpool_ret_buf(dpool, b);

}

int in_code(struct code_elem *h, void *buf, int len, void *pdata)
{
    int r;
    int c;
    struct dpool_buf *b;
    struct dpool *dpool = pdata;
    struct timeval tv; 

    printf("!! in\n");
    //c = getc(stdin);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    //for(;;){
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    r = select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if(r == -1){
        perror("select()");
        goto end;
    }else if(r){
        printf("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
        if(FD_ISSET(STDIN_FILENO, &fds)){
            int n;
            char buf[128];
            b = dpool_get_buf(dpool);
            if(!b){
                printf("!! couldn't get buff\n");
                return 1;
            }
            n = read(STDIN_FILENO, b->data, 128 - 1);

            code_out_avail(h, 0, b, n, cleanup_cb);
        }
    }else{
        printf("No data...\n");
        goto end;
    }


end:
        c = 9;
        //out->sendto = CODE_SENDTO_ALL;
       // code_sendto_all()
       // code_sendto_all_but_in()
       // code_sendto_no(int no, ids..)
    //}
}

int in_code2(struct code_elem *h, void *buf, int len, void *pdata)
{
    struct dpool_buf *b = buf;
    struct dpool *dpool = pdata;
    char *data = b->data;

    printf("Got buf: %s(%d)\n", data, len);
    dpool_ret_buf(dpool, b);

}

typedef enum {
    CODE_GRP_ONE = 0x01,
    CODE_GRP_TWO = 0x02
} CODE_GRP;

    struct dpool *dpool;

    struct code_net *net;
    struct code_elem *e0, *e1, *e2;
    struct code_grp *grp0;

    dpool = dpool_create(128, 8, 0);

    net = codenet_net_init();

    grp0 = codenet_grp_init(CODE_GRP_ONE);

    e0 = codenet_elem_init(CODE_TYPE_THREAD, CODE_ATTR_NO_INPUT, in_code, dpool);
    codenet_net_add_memb(net, e0);
    codenet_grp_add_memb(grp0, e0);


int main(int argc, char **argv)
{
    int r;

    struct dpool *dpool;

    struct cn_net *net;
    struct cn_elem *e0, *e1, *e2;
    struct cn_grp *grp0;

    dpool = dpool_create(128, 8, 0);

    /* create net */
    net = cn_net_init();
    OK(net);

    /* add threads that will control sending data between elem's */
    r = cn_io_add_ctrl(net, code_io_ctrl_default);
    OK(r);

    grp0 = code_grp_init(CODE_GRP_ONE);

    //grp0 = code_grp_get_memb(CODE_GRP_ONE);

    /* create elem's and add as memb to net and grp's */
    e0 = cn_elem_init(CODE_TYPE_THREAD, CODE_ATTR_NO_INPUT, in_code, dpool);
    cn_net_add_memb(net, e0);
    cn_grp_add_memb(grp0, e0);

    e1 = cn_init(CODE_TYPE_THREAD, 0, in_code2, dpool);
    cn_net_add_memb(net, e1);
    cn_grp_add_memb(grp0, e1);

    e2 = code_elem_init(CODE_TYPE_THREAD, 0, in_code2, dpool);
    code_net_add_memb(net, e1);

    /* connect elem's */
    code_net_conn(net, e0, e1);
    code_net_conn(net, e0, e2);

    /* run elem's */
    ce_elem_conn()
    code_elem_run(e0);
    code_elem_run(e1);
    code_elem_run(e2);

    /* run net */
    code_net_run(net);

    for(;;){
        sleep(3);
     //   code_tx_cmd(e0, 9, NULL);
     //   code_tx_cmd(e1, 9, NULL);
     //   code_tx_cmd(e2, 9, NULL);
    }

    return 0;

}


