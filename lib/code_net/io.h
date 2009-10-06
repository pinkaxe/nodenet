#ifndef __CN_IO_H__
#define __CN_IO_H__

enum {
    CN_SEND_TO_ELEM,
    CN_SEND_TO_GRP
};

struct cn_io_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/elem */
    int sendto_id;  /* depend on send_to_type grp_id/elem_id */
};

struct cn_io_cmd {
    enum cn_elem_cmd id;
    void *pdata;
    int data_no;
    struct cn_io_conf *conf;
};

struct cn_io_data {
    void *data;
    int data_no;
    struct cn_io_conf *conf;
};

typedef int (*io_cmd_req_cb_t)(struct cn_net *n, struct cn_io_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct cn_net *n, struct cn_io_data *data);

int cn_io_set_cmd_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_cmd
            *req));
int cn_io_add_cmd_req(struct cn_io_cmd *req);

int cn_io_set_data_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_data
            *data));
int cn_io_add_data_req(struct cn_io_data *req);

#endif
