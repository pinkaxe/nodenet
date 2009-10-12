#ifndef __NN_NODE_TYPES_H__
#define __NN_NODE_TYPES_H__

enum nn_node_type {
    NN_NODE_TYPE_THREAD,
    NN_NODE_TYPE_LPROC,
    NN_NODE_TYPE_BIN
};

enum nn_node_attr {
    NN_ATTR_NO_INPUT = 0x01
};

enum nn_node_cmd {
    NN_NODE_CMD_STOP
};

enum nn_node_state {
    NN_NODE_CMD_RUNNING,
    NN_NODE_CMD_PAUSED,
    NN_NODE_CMD_STOPPED,
};

struct nn_router;
struct nn_node;
struct nn_grp;
struct nn_grp_memb;

struct nn_cmd;
struct nn_io_data;

#endif
