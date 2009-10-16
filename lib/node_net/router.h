
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

/* these don't exist, just for type checking */
struct router_conn_iter;
struct node_conn_iter;

struct nn_router *router_init();
int router_free(struct nn_router *h);

int router_run(struct nn_router *h);

int router_lock(struct nn_router *rt);
int router_unlock(struct nn_router *rt);

int router_conn(struct nn_router *rt, struct nn_conn *cn);
int router_unconn(struct nn_router *h, struct nn_conn *n);
int router_isconn(struct nn_router *rt, struct nn_node *n);

int router_set_status(struct nn_router *rt, enum nn_state state);
enum nn_status router_get_status(struct nn_router *rt);

struct router_conn_iter *router_conn_iter_init(struct nn_router *rt);
int router_conn_iter_free(struct router_conn_iter *iter);
int router_conn_iter_next(struct router_conn_iter *iter, struct nn_conn **cn);


//int router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb);

#endif
