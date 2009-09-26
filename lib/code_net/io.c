
struct cn_io_data_req {
    struct code_elem *from; /* from who? */
    int type;  /* how to cleanup */
    void *func; /* func to call to cleanup */
    int id; /* eg. group id */
    void *buf;
};

struct cn_io_cmd_req {
    cmd_id;
}

/*
struct cn_io *io_init()
{
    int r;
    int err;
    struct cn_io *io;

    PCHK(LWARN, io, calloc(1, sizeof(*io)));
    if(!io){
    io    goto err;
    }
    DBG_STRUCT_INIT(io);
}

int io_free(struct cn_io *io)
{
}
*/

int io_add_data_req(struct code_net *net, struct code_elem *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

int io_add_cmd_req(struct code_net *net, struct code_elem *from, 
        void *buf, struct code_buf_prop *prop, int len)
{
}

static void io_ctrl_thread(void *arg)
{
    // loop and call code_io_ctrl_default for each req
}

int cn_io_ctrl_default(struct code_io_data_req *req)
{
    // check req and send appropriately
    switch(req->type){
        case ALL:
            ll_foreach(req->from->out_link_queh, curr, track){
                code_tx_data(curr->link, buf, len);
            }
            break;
        case NO:
            ll_foreach(req->from->out_link_queh, curr, track){
                code_tx_data(curr->link, buf, len);
                c++;
            }
            break;
        case GRP_ALL:
            break;
        case GRP_NO:
            break;
        default:
            break;
    }
    // remove back pointers
}
