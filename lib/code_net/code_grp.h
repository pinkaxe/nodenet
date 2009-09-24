#ifndef __CODE_GRP_H__
#define __CODE_GRP_H__

#include<stdbool.h>

struct code_grp;

struct code_grp *code_grp_init(int id);
void code_grp_free(struct code_grp *h);
int code_grp_add_memb(struct code_grp *h, void *memb);
bool code_grp_is_memb(struct code_grp *h, void *memb);
int code_grp_rem_memb(struct code_grp *h, void *memb);


#endif
