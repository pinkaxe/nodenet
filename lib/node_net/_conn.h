#ifndef _NN_CONN_H__
#define _NN_CONN_H__

/* private functions, should only be used
 * conn.c - to init and free new connections 
 * router.c - to conn/unconn from conn
 * node.c - to conn/unconn from conn
 */

struct nn_conn *_conn_init();
int _conn_free(struct nn_conn *cn);
int _conn_free_node(struct nn_conn *cn);
int _conn_free_router(struct nn_conn *cn);

int _conn_set_node(struct nn_conn *cn, struct nn_node *n);
int _conn_set_router(struct nn_conn *cn, struct nn_router *r);
//int conn_set_state(struct nn_conn *cn, enum nn_conn_state state);

int _conn_join_grp(struct nn_conn *cn, int grp_id);
int _conn_quit_grp(struct nn_conn *cn, int grp_id);

struct nn_node *_conn_get_node(struct nn_conn *cn);
struct nn_router *_conn_get_router(struct nn_conn *cn);
int _conn_get_state(struct nn_conn *cn);
int _conn_get_grp_id(struct nn_conn *cn);

/* connections between routers and nodes */
int _conn_conn(struct nn_node *n, struct nn_router *rt);
int _conn_unconn(struct nn_node *n, struct nn_router *rt);

/* router -> node pkt */
int _conn_node_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);
int _conn_node_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt);
int _conn_router_tx_pkt(struct nn_conn *cn, struct nn_pkt *pkt);
int _conn_router_rx_pkt(struct nn_conn *cn, struct nn_pkt **pkt);

int _conn_set_rx_cnt(struct nn_conn *cn, int grp_id, int cnt);

#endif
