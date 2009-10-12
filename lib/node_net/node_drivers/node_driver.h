#ifndef NN_NODE_DRIVERS_NODE_DRIVER_H__
#define NN_NODE_DRIVERS_NODE_DRIVER_H__

struct node_driver_ops {
    int (*node_buf_exe)(struct nn_node *n, char *buf, size_t len, void *pdata);
    int (*node_timer_exe)(struct nn_node *n);
    //int (*node_cmd_check)(struct nn_node *n, struct nn_io_cmd **cmd);
    //int (*node_data_check)(struct nn_node *n, struct nn_io_data **cmd);
};


struct node_driver_ops *node_driver_get_ops(int type);

#endif
