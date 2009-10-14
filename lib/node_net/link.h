#ifndef NN_LINK_H__
#define NN_LINK_H__


struct nn_link *link_init();
int link_free(struct nn_link *l);

int link_link(struct nn_link *l, struct nn_node *n, struct nn_router *rt);
int link_unlink(struct nn_node *n, struct nn_router *rt);


/* router -> node cmd */
int link_router_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd);
int link_node_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd);

/* router -> node data */
//int link_router_tx_data(struct nn_router *rt, struct nn_data *data);
//int link_node_rx_data(struct nn_router *rt, struct nn_data **data);

/* node -> router cmd */
int link_node_tx_cmd(struct nn_router *rt, struct nn_cmd *cmd);
int link_router_rx_cmd(struct nn_router *rt, struct nn_cmd **cmd);

/* node -> router data */
//int link_node_tx_data(struct nn_router *rt, struct nn_data *data);
//int link_router_rx_data(struct nn_router *rt, struct nn_data **data);

/* router -> node notify */
//int link_node_tx_notify(struct nn_router *rt, struct nn_notify *notify);
//int link_router_rx_notify(struct nn_router *rt, struct nn_notify **notify)
//


#endif
