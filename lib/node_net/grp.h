
#ifndef NN_GRP_H__
#define NN_GRP_H__

#include<stdbool.h>

struct nn_grp;
struct grp_node_iter;

struct nn_grp *grp_init(int id);
int grp_free(struct nn_grp *h);

int grp_add_node(struct nn_grp *h, struct nn_node *memb);
int grp_rem_node(struct nn_grp *h, struct nn_node *memb);
int grp_ismemb(struct nn_grp *h, struct nn_node *memb);

struct grp_node_iter *grp_node_iter_init(struct nn_grp *g);
int grp_node_iter_free(struct grp_node_iter *iter);
int grp_node_iter_next(struct grp_node_iter *iter, struct nn_conn **cn);

int grp_lock(struct nn_grp *g);
int grp_unlock(struct nn_grp *g);

#endif
