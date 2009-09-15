
typedef enum {
   code_t_thread,
   code_t_process,
   code_t_shell,
} code_t;


void *code_t_thread_thread(void *arg)
{
    // Get stuff from arg
    func(in_buf, in_buf_len, out_buf, out_buf_len);
}

struct code {

    code_t type;
    void *code;

    unsigned int in_buf_size;
    void *in_buf;

    unsigned int out_buf_size;
    void *out_buf;

    unsigned int out_code_no;
    struct code *next[4];

};

struct code_net {
    struct code *roots;
};



struct code *code_create(type, int in_buf_size,, int out_buf_size, )
code_free();
code_link(net, in, out, int out_buf_size, )
code_unlink(net, in, out, int out_buf_size, )


int code_net_set_ev_cb()
{
}


code_net_walk(struct code_net *root)
{
    //struct code *code;

    for(;;){
        code_run();
    }
}


/* app */

void *in_code(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    int c;

    for(;;){
        c = getc(stdin);

        b = get_free_out_buf();
        b[0] = c;
        signal;
    }

}

void *code_upper(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    for(;;){
        wait_for_input();

        b = get_free_out_buf();

        b[0] = toupper(in_buf[0]);
        get_free_ret_buf();
        fill;
        signal;

    }

}

void *out_code(void *in_buf, int in_buf_size, void *out_buf, int out_code_no)
{
    for(;;){
        wait_for_input();
        printf;
    }
}


int main(int argc, char const* argv[])
{
    struct code_net;
    struct code *e0, *e1, *e2;

    e0 = code_create(0, 128, code_t_thread, in_code);

    //e1 = code_create(128, 128, code_t_shell, "test.sh");
    //code_conn(code_net, e0, e1);
    e1 = code_create(128, 128, code_t_thread, code_upper);
    code_link(code_net, e0, e1, buf0, link0_cb);

    e2 = code_create(128, 0, code_t_thread, out_code);
    cl1 = code_link(code_net, e1, e2, buf1, link1_cb);

    link_snoop(cl1)

    code_net_set_ev_cb();

    code_net_run();

    return 0;
}




