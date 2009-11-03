#ifndef NN_NODE_H__
#define NN_NODE_H__

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
/* node_free only set state to shutdown, node_clean has to be called to do the
 * final cleanup */
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

/* iter */
struct node_conn_iter *node_conn_iter_init(struct nn_node *rt);
int node_conn_iter_free(struct node_conn_iter *iter);
int node_conn_iter_next(struct node_conn_iter *iter, struct nn_conn **cn);

/* locking */
int node_lock(struct nn_node *n);
int node_unlock(struct nn_node *n);
int node_cond_wait(struct nn_node *n);
int node_cond_broadcast(struct nn_node *n);

int node_print(struct nn_node *n);

int node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt);
int node_add_rx_pkt(struct nn_node *n, struct nn_pkt *pkt);
int node_get_tx_pkt(struct nn_node *n, struct nn_pkt **pkt);
int node_get_rx_pkt(struct nn_node *n, struct nn_pkt **pkt);

/* easy iterator pre/post
 * n != NULL when this is called, afterwards cn for
 each matching router is set and can be used */
#define NODE_CONN_ITER_PRE \
    assert(n); \
    int done = 0; \
    struct node_conn_iter *iter; \
    struct nn_conn *cn; \
    node_lock(n); \
    iter = node_conn_iter_init(n); \
    while(!done && !node_conn_iter_next(iter, &cn)){ \
        conn_lock(cn);

#define NODE_CONN_ITER_POST \
        conn_unlock(cn); \
    } \
    node_conn_iter_free(iter); \
    node_unlock(n);


#endif
