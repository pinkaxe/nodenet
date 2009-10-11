#ifndef __CN_IO_H__
#define __CN_IO_H__

#include "code_net/types.h"

enum {
    CN_SENDTO_ELEM,
    CN_SENDTO_GRP,
    CN_SENDTO_ALL
};

/* FIXME: move to .c and implement getters */
struct cn_io_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/elem/all */
    int sendto_id;  /* depend on send_to_type grp_id/elem_id */
};

struct cn_cmd {
    enum cn_elem_cmd id;
    void *pdata;
    int data_no;
    struct cn_io_conf *conf;
};

/*
struct cn_io_data {
    void *data;
    int data_no;
    struct cn_io_conf *conf;
};
*/


typedef int (*io_cmd_req_cb_t)(struct cn_router *rt, struct cn_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct cn_router *rt, struct cn_io_data *data);

/*
int cn_io_set_cmd_cb(struct cn_router *rt, int (*cn_io_req)(struct cn_cmd
            *req));
int cn_io_add_cmd_req(struct cn_cmd *req);

int cn_io_set_data_cb(struct cn_router *rt, int (*cn_io_req)(struct cn_io_data
            *data));
int cn_io_add_data_req(struct cn_io_data *req);
*/

struct cn_cmd *cmd_init(enum cn_elem_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id);
int cmd_free(struct cn_cmd *cmd);
struct cn_cmd *cmd_clone(struct cn_cmd *cmd);

#endif
