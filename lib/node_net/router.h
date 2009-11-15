
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

struct nn_router *router_init();
int router_free(struct nn_router *rt);
int router_clean(struct nn_router *rt);

int router_add_grp(struct nn_router *rt, int id);
/* return -1 no such group */
int router_rem_grp(struct nn_router *rt, int id);
struct nn_grp *router_get_grp(struct nn_router *rt, int id);

int router_conn(struct nn_router *rt, struct nn_conn *cn);
int router_unconn(struct nn_router *rt, struct nn_conn *n);
int router_isconn(struct nn_router *rt, struct nn_node *n);

enum nn_state router_get_state(struct nn_router *rt);
int router_set_state(struct nn_router *rt, enum nn_state state);

int router_print(struct nn_router *rt);

int router_add_to_grp(struct nn_router *rt, int grp_id, struct nn_conn *cn);
int router_rem_from_grp(struct nn_router *rt, int grp_id, struct nn_conn *cn);

#endif
