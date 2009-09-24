
#ifndef __CN_GRP_H__
#define __CN_GRP_H__

#include<stdbool.h>

struct cn_grp;

struct cn_grp *grp_init(int id);
int grp_free(struct cn_grp *h);

int grp_add_memb(struct cn_grp *h, struct cn_elem *memb);
int grp_rem_memb(struct cn_grp *h, void *memb);
bool grp_is_memb(struct cn_grp *h, void *memb);


#endif
