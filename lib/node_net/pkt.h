#ifndef NN_PKT_H__
#define NN_PKT_H__

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


struct nn_packet_pkt {
    enum nn_pkt_pkt id;
    void *pdata;
    int data_no;
};


typedef int (*io_pkt_req_cb_t)(struct nn_router *rt, struct nn_pkt *pkt);
typedef int (*io_data_req_cb_t)(struct nn_router *rt, struct nn_io_data *data);


struct nn_pkt *pkt_init(enum nn_pkt_pkt id, void *pdata, int data_no,
        int sendto_no, int sendto_type, int sendto_id);
int pkt_free(struct nn_pkt *pkt);
struct nn_pkt *pkt_clone(struct nn_pkt *pkt);

enum nn_pkt_pkt pkt_get_id(struct nn_pkt *pkt);

#endif
