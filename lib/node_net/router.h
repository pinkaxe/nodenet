
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

struct nn_router *router_init();
int router_free(struct nn_router *h);

int router_run(struct nn_router *h);

int router_lock(struct nn_router *rt);
int router_unlock(struct nn_router *rt);

int router_conn(struct nn_router *rt, struct nn_link *l);
int router_dconn(struct nn_router *h, struct nn_link *n);
int router_isconn(struct nn_router *rt, struct nn_node *n);
struct nn_link *router_conn_iter(struct nn_router *rt, void **iter);

int router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb);

#endif
