#ifndef NN_TYPES_H__
#define NN_TYPES_H__

struct nn_router;
struct nn_node;
struct nn_chan;
struct nn_chan_rel;

struct nn_pkt;
struct nn_io_data;

struct nn_conn;


/* FIXME: some of these enum's can be moved to there .c files .h */
enum nn_node_driver {
    NN_NODE_TYPE_THREAD,
    NN_NODE_TYPE_LPROC,
    NN_NODE_TYPE_BIN
};

enum nn_node_attr {
    NN_NODE_ATTR_NO_INPUT = 0x01
};

enum nn_node_state {
    NN_NODE_STATE_RUNNING,
    NN_NODE_STATE_PAUSED,
    NN_NODE_STATE_STOPPED,
};

enum nn_pkt_pkt {
    NN_pkt_pkt_STOP
};

enum nn_state {
    NN_STATE_RUNNING,
    NN_STATE_PAUSED,
    NN_STATE_SHUTDOWN,
    NN_STATE_DONE,
};

enum {
    NN_RET_NODE_DONE = 0x7F,
};

#endif
