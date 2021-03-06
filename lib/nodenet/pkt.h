#ifndef NN_PKT_H__
#define NN_PKT_H__

#include "nodenet/types.h"

enum {
    NN_SENDTO_NODE,
    NN_SENDTO_GRP,
    NN_SENDTO_ALL
};

enum nn_pkt_state {
    PKT_STATE_N_INIT,
    PKT_STATE_N_TX,
    PKT_STATE_RT_RX,
    PKT_STATE_RT_ROUTED,
    PKT_STATE_N_RX,
    PKT_STATE_CANCELLED,
};

/* FIXME: move to .c and implement getters */
struct nn_pkt_conf {
    int sendto_no;  /* send to how many */
    int sendto_type; /* grp/node/all */
    int sendto_id;  /* depends on sendto_type grp_id/node_id */
};


struct nn_packet_pkt {
    enum nn_pkt_pkt id;
    void *pdata;
    int data_no;
};

/* cb to free packet buffer */
typedef int (*buf_free_cb_f)(void *pdata, void *buf);

typedef int (*io_pkt_req_cb_t)(struct nn_router *rt, struct nn_pkt *pkt);
typedef int (*io_data_req_cb_t)(struct nn_router *rt, struct nn_io_data *data);

struct nn_pkt *pkt_init(struct nn_node *src, struct nn_chan *ch, int dest_no,
        void *data, int data_len, void *pdata, buf_free_cb_f buf_free_cb);
int pkt_free(struct nn_pkt *pkt);
struct nn_pkt *pkt_clone(struct nn_pkt *pkt);

int pkt_set_state(struct nn_pkt *pkt, enum nn_pkt_state state);
enum nn_pkt_state pkt_get_state(struct nn_pkt *pkt);

struct nn_chan *pkt_get_dest_chan(struct nn_pkt *pkt);
int pkt_get_dest_no(struct nn_pkt *pkt);
struct nn_node *pkt_get_src(struct nn_pkt *pkt);

enum nn_pkt_pkt pkt_get_id(struct nn_pkt *pkt);
void *pkt_get_data(struct nn_pkt *pkt);
int pkt_get_data_len(struct nn_pkt *pkt);
void *pkt_get_pdata(struct nn_pkt *pkt);
int pkt_set_src(struct nn_pkt *pkt, struct nn_node *n);

int pkt_inc_refcnt(struct nn_pkt *pkt, int inc);
int pkt_dec_refcnt(struct nn_pkt *pkt, int dec);

int pkt_cancelled(struct nn_pkt *pkt);
#endif
