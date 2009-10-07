#ifndef __CN_IO_H__
#define __CN_IO_H__

#include "code_net/types.h"

enum {
    CN_SENDTO_ELEM,
    CN_SENDTO_GRP,
    CN_SENDTO_ALL
};

typedef int (*io_cmd_req_cb_t)(struct cn_net *n, struct cn_io_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct cn_net *n, struct cn_io_data *data);

/*
int cn_io_set_cmd_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_cmd
            *req));
int cn_io_add_cmd_req(struct cn_io_cmd *req);

int cn_io_set_data_cb(struct cn_net *n, int (*cn_io_req)(struct cn_io_data
            *data));
int cn_io_add_data_req(struct cn_io_data *req);
*/

struct cn_io_cmd *io_cmd_init(enum cn_elem_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id);
int io_cmd_free(struct cn_io_cmd *cmd);

#endif
