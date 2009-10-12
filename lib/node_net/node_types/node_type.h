#ifndef node_TYPE_THREAD_H__
#define node_TYPE_THREAD_H__

struct node_type_ops {
    int (*node_buf_exe)(struct nn_node *n, char *buf, size_t len, void *pdata);
    int (*node_timer_exe)(struct nn_node *n);
    //int (*node_cmd_check)(struct nn_node *n, struct nn_io_cmd **cmd);
    //int (*node_data_check)(struct nn_node *n, struct nn_io_data **cmd);
};


struct node_type_ops *node_type_get_ops(int type);

#endif
