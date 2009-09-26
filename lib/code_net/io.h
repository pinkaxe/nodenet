#ifndef __CN_IO_H__
#define __CN_IO_H__

typedef int (*io_cmd_req_cb_t)(struct cn_net *n, struct cn_io_cmd_req *req);
typedef int (*io_data_req_cb_t)(struct cn_net *n, struct cn_io_data_req *req);

int cn_io_set_cmd_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_cmd_req
            *req));
int cn_io_add_cmd_req(struct cn_io_cmd_req *req);

#endif
