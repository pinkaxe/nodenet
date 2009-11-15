#ifndef NN_CONN_H__
#define NN_CONN_H__

struct nn_conn *conn_conn(struct nn_node *n, struct nn_router *rt);
int conn_unconn(struct nn_node *n, struct nn_router *rt);

int conn_join_grp(struct nn_conn *cn, int grp_id);
int conn_quit_grp(struct nn_conn *cn, int grp_id);

#endif
