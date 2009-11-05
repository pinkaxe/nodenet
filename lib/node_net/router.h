
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

/* these types don't exist, just for type checking */
struct router_conn_iter;
struct node_conn_iter;

struct nn_router *router_init();
int router_free(struct nn_router *rt);
int router_clean(struct nn_router *rt);

int router_io_run(struct nn_router *rt);

int router_lock(struct nn_router *rt);
int router_unlock(struct nn_router *rt);

int router_conn(struct nn_router *rt, struct nn_conn *cn);
int router_unconn(struct nn_router *rt, struct nn_conn *n);
int router_isconn(struct nn_router *rt, struct nn_node *n);

int router_set_status(struct nn_router *rt, enum nn_state state);
enum nn_status router_get_status(struct nn_router *rt);

struct router_conn_iter *router_conn_iter_init(struct nn_router *rt);
int router_conn_iter_free(struct router_conn_iter *iter);
int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **cn);

int router_conn_each(struct nn_router *rt,
        int (*cb)(struct nn_conn *cn, void *a0), 
        struct nn_pkt *pkt);

enum nn_state router_get_state(struct nn_router *rt);
int router_set_state(struct nn_router *rt, enum nn_state state);

//int router_lock(struct nn_router *rt);
//int router_unlock(struct nn_router *rt);
//int router_cond_wait(struct nn_router *rt);
//int router_cond_broadcast(struct nn_router *rt);

int router_print(struct nn_router *rt);

int router_rx_pkts(struct nn_router *rt);
int router_get_rx_pkt(struct nn_router *rt, struct nn_pkt **pkt);

int router_add_tx_pkt(struct nn_router *rt, struct nn_pkt *pkt);
int router_tx_pkts(struct nn_router *rt);


#endif
