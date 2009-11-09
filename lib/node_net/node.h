#ifndef NN_NODE_H__
#define NN_NODE_H__

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int node_free(struct nn_node *node);
int node_clean(struct nn_node *n);

/* conn to routers via conns */
int node_conn(struct nn_node *n, struct nn_conn *cn);
int node_unconn(struct nn_node *n, struct nn_conn *cn);

/* grp memb. */
int node_join_grp(struct nn_node *n, struct nn_grp *g);
int node_quit_grp(struct nn_node *n, struct nn_grp *g);

/* getters */
int node_get_type(struct nn_node *n);
int node_get_attr(struct nn_node *n);
void *node_get_pdatap(struct nn_node *n);
void *node_get_codep(struct nn_node *n);
enum nn_state node_get_state(struct nn_node *n);
struct nn_conn *node_get_router_conn(struct nn_node *n, struct nn_router *rt);

/* setters */
int node_set_state(struct nn_node *n, enum nn_state state);

/* node api */
int node_do_state(struct nn_node *n);
//int node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt);
int node_get_rx_pkt(struct nn_node *n, struct nn_pkt **pkt);

int node_tx(struct nn_node *n, void *data, int data_len, void *pdata, int
        sendto_no, int sendto_type, int sendto_id, buf_free_cb_f buf_free_cb);

int node_rx(struct nn_node *n, void **data, int *data_len, void **pdata);

int node_print(struct nn_node *n);

#endif
