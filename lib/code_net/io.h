#ifndef __CN_IO_H__
#define __CN_IO_H__

enum {
    CN_SEND_TO_ELEM,
    CN_SEND_TO_GRP
};

struct cn_cmd {
    enum cn_elem_cmd id;
    void *pdata;
    int data_no;
    int send_to_type; /* grp/elem */
    int send_to_id;  /* depend on send_to_type grp_id/elem_id */
    int send_to_no;  /* send to how many */
};

typedef int (*io_cmd_req_cb_t)(struct cn_net *n, struct cn_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct cn_net *n, struct cn_io_data_req *req);

int cn_io_set_cmd_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_cmd_req
            *req));
int cn_io_add_cmd_req(struct cn_io_cmd_req *req);

#endif
