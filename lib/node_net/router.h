
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

struct nn_router *router_init();
int router_free(struct nn_router *h);

int router_run(struct nn_router *h);

int router_lock(struct nn_router *rt);
int router_unlock(struct nn_router *rt);

int router_add_memb(struct nn_router *rt, struct nn_conn_node_router *cn);
int router_rem_memb(struct nn_router *h, struct nn_conn_node_router *n);
int router_ismemb(struct nn_router *rt, struct nn_node *n);
struct nn_node *router_memb_iter(struct nn_router *rt, void **iter);

int router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb);

int router_add_cmd(struct nn_router *rt, struct nn_cmd *cmd);
struct nn_cmd *router_get_cmd(struct nn_router *rt, struct timespec *ts);


#endif
