#ifndef NN_CONN_H__
#define NN_CONN_H__

#define NN_CONN_STATE_DEAD 1

struct nn_conn *conn_init();
int conn_free(struct nn_conn *cn);
int conn_free_node(struct nn_conn *cn);
int conn_free_router(struct nn_conn *cn);

int conn_set_node(struct nn_conn *cn, struct nn_node *n);
int conn_set_router(struct nn_conn *cn, struct nn_router *rt);
//int conn_set_state(struct nn_conn *cn, enum nn_conn_state state);

struct nn_node *conn_get_node(struct nn_conn *cn);
struct nn_router *conn_get_router(struct nn_conn *cn);
int conn_get_state(struct nn_conn *cn);


/* router -> node pkt */
int conn_router_tx_pkt(struct nn_router *rt, struct nn_node *n, struct nn_pkt
        *pkt);
int conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);

int conn_node_tx_pkt(struct nn_node *n, struct nn_router *rt, struct nn_pkt
        *pkt);
int conn_router_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);

int conn_lock(struct nn_conn *cn);
int conn_unlock(struct nn_conn *cn);


#endif
