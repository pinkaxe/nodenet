#ifndef NN_CONN_H__
#define NN_CONN_H__

#define NN_CONN_STATE_DEAD 1

struct nn_conn *conn_init();
int conn_free_node(struct nn_conn *cn);
int conn_free_router(struct nn_conn *cn);

int conn_set_node(struct nn_conn *cn, struct nn_node *n);
int conn_set_router(struct nn_conn *cn, struct nn_router *rt);
//int conn_set_state(struct nn_conn *cn, enum nn_conn_state state);

struct nn_node *conn_get_node(struct nn_conn *cn);
struct nn_router *conn_get_router(struct nn_conn *cn);
int conn_get_state(struct nn_conn *cn);


//int conn_conn(struct nn_conn *cn, struct nn_node *n, struct nn_router *rt);
//int conn_unconn(struct nn_node *n, struct nn_router *rt);


/* router -> node cmd */
int conn_router_tx_icmd(struct nn_router *rt, struct nn_node *n, struct nn_icmd
        *icmd);
int conn_node_rx_icmd(struct nn_node *n, struct nn_router *rt, struct nn_icmd
        **icmd);

/* node -> router cmd */
//int conn_node_tx_cmd(struct nn_node *rt, struct nn_cmd *cmd);
//int conn_router_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd);

/* router -> node data */
//int conn_router_tx_data(struct nn_router *rt, struct nn_data *data);
//int conn_node_rx_data(struct nn_router *rt, struct nn_data **data);

/* node -> router data */
//int conn_node_tx_data(struct nn_router *rt, struct nn_data *data);
//int conn_router_rx_data(struct nn_router *rt, struct nn_data **data);

/* router -> node notify */
//int conn_node_tx_notify(struct nn_router *rt, struct nn_notify *notify);
//int conn_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
//


#endif
