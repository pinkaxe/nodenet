#ifndef NN_TYPES_H__
#define NN_TYPES_H__

struct nn_router;
struct nn_node;
struct nn_grp;
struct nn_grp_memb;

struct nn_cmd;
struct nn_io_data;

struct nn_link;

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

enum nn_cmd_cmd {
    NN_CMD_CMD_STOP
};

#endif
