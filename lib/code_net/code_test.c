
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include "code_net/code.h"
#include "util/dpool.h"

void sending_to_no_cb(void *buf, int no)
{
   struct dpool_buf *b = buf;
   // // FIXME: do through function
   b->ref_cnt = no;
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

            code_out_avail(h, 0, b, n, sending_to_no_cb);
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

int main(int argc, char **argv)
{
    struct code_elem *e0, *e1, *e2;
    struct dpool *dpool;

    dpool = dpool_create(128, 8, 0);

    e0 = code_create(CODE_TYPE_THREAD, CODE_ATTR_NO_INPUT, in_code, dpool);
    e1 = code_create(CODE_TYPE_THREAD, 0, in_code2, dpool);
    e2 = code_create(CODE_TYPE_THREAD, 0, in_code2, dpool);

    code_link(e0, e1);
    code_link(e0, e2);

    code_run(e0);
    code_run(e1);
    code_run(e2);

    for(;;){
        sleep(3);
     //   code_tx_cmd(e0, 9, NULL);
     //   code_tx_cmd(e1, 9, NULL);
     //   code_tx_cmd(e2, 9, NULL);
    }

    return 0;

}


