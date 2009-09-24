#ifndef __CN_H__
#define __CN_H__

struct cn_net *cn_net_init();
int cn_net_free(struct cn_net *n);
//code_net_run

struct cn_elem *cn_elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata);
int cn_elem_free(struct cn_elem *e);
//code_elem_run

struct cn_grp *cn_grp_init(int id);
int cn_grp_free(struct cn_grp *g);

int cn_add_elem_to_net();
int cn_rem_elem_from_net();

int cn_add_elem_to_grp();
int cn_rem_elem_from_grp();

int cn_link_elem(struct cn_elem *from, struct cn_elem *to);
int cn_unlink_elem(struct cn_elem *from, struct cn_elem *to);

#endif
