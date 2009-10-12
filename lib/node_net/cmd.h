#ifndef __nn_IO_H__
#define __nn_IO_H__

#include "node_net/types.h"

enum {
    nn_SENDTO_node,
    nn_SENDTO_GRP,
    nn_SENDTO_ALL
};

/* FIXME: move to .c and implement getters */
struct nn_io_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/node/all */
    int sendto_id;  /* depend on send_to_type grp_id/node_id */
};

struct nn_cmd {
    enum nn_node_cmd id;
    void *pdata;
    int data_no;
    struct nn_io_conf *conf;
};

/*
struct nn_io_data {
    void *data;
    int data_no;
    struct nn_io_conf *conf;
};
*/


typedef int (*io_cmd_req_cb_t)(struct nn_router *rt, struct nn_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct nn_router *rt, struct nn_io_data *data);

/*
int nn_io_set_cmd_cb(struct nn_router *rt, int (*nn_io_req)(struct nn_cmd
            *req));
int nn_io_add_cmd_req(struct nn_cmd *req);

int nn_io_set_data_cb(struct nn_router *rt, int (*nn_io_req)(struct nn_io_data
            *data));
int nn_io_add_data_req(struct nn_io_data *req);
*/

struct nn_cmd *cmd_init(enum nn_node_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id);
int cmd_free(struct nn_cmd *cmd);
struct nn_cmd *cmd_clone(struct nn_cmd *cmd);

#endif
