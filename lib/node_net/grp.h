
#ifndef NN_GRP_H__
#define NN_GRP_H__

#include<stdbool.h>

struct nn_grp;
struct grp_node_iter;

struct nn_grp *grp_init(int id);
int grp_free(struct nn_grp *h);

int grp_add_grp_rel(struct nn_grp *g, struct nn_grp_rel *grp_rel);
int grp_rem_grp_rel(struct nn_grp *h, struct nn_grp_rel *grp_rel);
int grp_ismemb(struct nn_grp *h, struct nn_node *memb);

int grp_lock(struct nn_grp *g);
int grp_unlock(struct nn_grp *g);

#endif
