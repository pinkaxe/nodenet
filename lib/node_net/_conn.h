#ifndef _NN_CONN_H__
#define _NN_CONN_H__

struct nn_conn *_conn_init();
int _conn_free(struct nn_conn *cn);
int _conn_free_node(struct nn_conn *cn);
int _conn_free_router(struct nn_conn *cn);

int _conn_set_node(struct nn_conn *cn, struct nn_node *n);
int _conn_set_router(struct nn_conn *cn, struct nn_router *rt);
//int conn_set_state(struct nn_conn *cn, enum nn_conn_state state);

struct nn_node *_conn_get_node(struct nn_conn *cn);
struct nn_router *_conn_get_router(struct nn_conn *cn);
int _conn_get_state(struct nn_conn *cn);

/* connections between routers and nodes */
int _conn_conn(struct nn_node *n, struct nn_router *rt);
int _conn_unconn(struct nn_node *n, struct nn_router *rt);

/* router -> node pkt */
int _conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);
int _conn_node_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt);
int _conn_router_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt);
int _conn_router_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);

int _conn_lock(struct nn_conn *cn);
int _conn_unlock(struct nn_conn *cn);

#endif
