#ifndef NN_NODE_DRIVERS_NODE_DRIVER_H__

struct node_driver_ops {
    /* add and get buf from driver */
    int (*put_buf)(struct nn_node *n, void *buf, size_t len, void *pdata);
    int (*get_buf)(struct nn_node *n, void **buf, size_t *len);
};


struct node_driver_ops *node_driver_get_ops(int type);

#endif
