#ifndef NN_CONN_H__
#define NN_CONN_H__

struct nn_conn {
    struct nn_node *n;
    struct nn_router *rt;
    mutex_t mutex;

    /* router(output) -> node(input) */
    struct que *rt_n_cmd;   /* router write node cmd */
    struct que *rt_n_data;  /* router write node data */

    /* node(output) -> router(input) */
    struct que *n_rt_notify; /* always used */
    struct que *n_rt_cmd;    /* only master node */
    struct que *n_rt_data;   /* n write data */

    /* internal going to and from router commands */
    struct que *n_rt_int_cmd;
    struct que *rt_n_int_cmd;
};

/* router -> node cmd */
int conn_router_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd);
int conn_node_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd);

/* router -> node data */
//int conn_router_tx_data(struct nn_router *rt, struct nn_data *data);
//int conn_node_rx_data(struct nn_router *rt, struct nn_data **data);

/* node -> router cmd */
int conn_node_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd);
int conn_router_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd);

/* node -> router data */
//int conn_node_tx_data(struct nn_router *rt, struct nn_data *data);
//int conn_router_rx_data(struct nn_router *rt, struct nn_data **data);

/* router -> node notify */
//int conn_node_tx_notify(struct nn_router *rt, struct nn_notify *notify);
//int conn_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
//
int conn_break_node_router(struct nn_node *n, struct nn_router *rt);

struct nn_conn *conn_init();

#endif
