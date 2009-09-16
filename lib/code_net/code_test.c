
#include <stdio.h>
#include "code_net/code.h"
#include "util/dpool.h"

static struct dpool *dpool;
static struct dpool_buf *filled_buf[8];

int in_code(struct code_elem *h) //void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    static int i = 0;
    static int c;
    static struct dpool_buf *buf;
    printf("!! in\n");

    c = getc(stdin);

    buf = dpool_get_buf(dpool);
    if(!buf){
        printf("!! couldn't get buff\n");
        return 1;
    }
    char *b = buf->data;
    b[0] = c;
    filled_buf[i++] = buf;
    code_out_avail(h, 0, buf->data, 1);

    //b = get_free_out_buf();
    //signal;
}

int in_code2(struct code_elem *h) //void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int i = 0;
    int c;
    struct dpool_buf *buf;

    for(;;){
        //sleep(1);
        //buf = dpool_get_filled_buf();
        code_wait(h);
        printf("!! great\n");
        if(filled_buf[i]){
            char *data = filled_buf[i]->data;
            printf("Got data: %c\n", data[0]);
x
            //dpool_ret_buf(dpool, filled_buf[i++]);
        }
    }
}


int main(int argc, char **argv)
{
    struct code_net;
    struct code_elem *e0, *e1, *e2;

    dpool = dpool_create(128, 8, 0);

    e0 = code_create(code_t_thread, in_code);
    code_run(e0);

    e1 = code_create(code_t_thread, in_code2);
    code_run(e1);

    e2 = code_create(code_t_thread, in_code2);
    code_run(e2);

    code_link(e0, e1);
    code_link(e0, e2);

    for(;;){
    }
}


