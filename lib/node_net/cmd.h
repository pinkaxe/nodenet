#ifndef NN_CMD_H__
#define NN_CMD_H__

#include "node_net/types.h"

enum {
    NN_SENDTO_NODE,
    NN_SENDTO_GRP,
    NN_SENDTO_ALL
};

/* FIXME: move to .c and implement getters */
struct nn_io_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/node/all */
    int sendto_id;  /* depend on send_to_type grp_id/node_id */
};

//struct nn_cmd {
//    enum nn_cmd_cmd id;
//    void *pdata;
//    int data_no;
//    struct nn_io_conf *conf;
//    //uint32_t seq_no;
//};

struct nn_packet_cmd {
    enum nn_cmd_cmd id;
    void *pdata;
    int data_no;
};


typedef int (*io_cmd_req_cb_t)(struct nn_router *rt, struct nn_cmd *cmd);
typedef int (*io_data_req_cb_t)(struct nn_router *rt, struct nn_io_data *data);


struct nn_cmd *cmd_init(enum nn_cmd_cmd id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id);
int cmd_free(struct nn_cmd *cmd);
struct nn_cmd *cmd_clone(struct nn_cmd *cmd);

#endif
