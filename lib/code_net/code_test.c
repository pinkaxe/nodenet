
#include <stdio.h>
#include "code_net/code.h"

void in_code() //void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int c;
    printf("!! in\n");

    for(;;){
        c = getc(stdin);
        printf("!!%c\n", toupper(c));

        //b = get_free_out_buf();
        //b[0] = c;
        //signal;
    }

}

int main(int argc, char const* argv[])
{
    struct code_net;
    struct code_elem *e0, *e1, *e2;

    e0 = code_create(code_t_thread, in_code);
    code_run(e0);

    while(1){
    }

}

