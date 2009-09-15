
#include <stdio.h>
#include "code_net/code.h"
#include "util/dpool.h"

static struct dpool *dpool;
static struct dpool_buf *filled_buf[8];

int in_code(struct code_elem *h) //void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int i = 0;
    int c;
    struct dpool_buf *buf;
    printf("!! in\n");

    for(;;){
        c = getc(stdin);
        printf("!!%c\n", toupper(c));
        buf = dpool_get_buf(dpool);
        if(!buf){
            printf("!! couldn't get buff\n");
            continue;
        }
        char *b = buf->data;
        b[0] = c;
        filled_buf[i++] = buf;
        code_out_avail(h, 0, buf, 1);

        //b = get_free_out_buf();
        //signal;
    }
}

int in_code2(struct code_elem *h) //void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int i = 0;
    int c;
    struct dpool_buf *buf;
    while(1){
        //sleep(1);
        //buf = dpool_get_filled_buf();
        //printf("!! great\n");
        code_wait(h);
        printf("fin wait\n");
        if(filled_buf[i]){
            char *data = filled_buf[i]->data;
            printf("Got data: %c\n", data[0]);
            dpool_ret_buf(dpool, filled_buf[i++]);
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

    code_link(e0, e1);

    for(;;){
    }
}


