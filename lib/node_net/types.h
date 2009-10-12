#ifndef __nn_TYPES_H__
#define __nn_TYPES_H__

enum nn_node_type {
    nn_TYPE_THREAD,
    nn_TYPE_LPROC,
    nn_TYPE_BIN
};

enum nn_node_attr {
    nn_ATTR_NO_INPUT = 0x01
};

enum nn_node_cmd {
    nn_node_CMD_STOP
};

enum nn_node_state {
    nn_node_CMD_RUNNING,
    nn_node_CMD_PAUSED,
    nn_node_CMD_STOPPED,
};

struct nn_router;
struct nn_node;
struct nn_grp;
struct nn_grp_memb;

struct nn_cmd;
struct nn_io_data;

#endif
