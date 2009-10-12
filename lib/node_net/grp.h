
#ifndef __nn_GRP_H__
#define __nn_GRP_H__

#include<stdbool.h>

struct nn_grp;

struct nn_grp *grp_init(int id);
int grp_free(struct nn_grp *h);

int grp_add_memb(struct nn_grp *h, struct nn_node *memb);
int grp_rem_memb(struct nn_grp *h, struct nn_node *memb);
int grp_ismemb(struct nn_grp *h, struct nn_node *memb);


#endif
