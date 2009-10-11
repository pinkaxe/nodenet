#ifndef __CN_ELEM_H__
#define __CN_ELEM_H__

struct cn_elem *elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata);
int elem_free(struct cn_elem *elem);

/* router memb. */
int elem_add_to_router(struct cn_elem *e, struct cn_router *rt);
int elem_rem_from_router(struct cn_elem *e, struct cn_router *rt);

/* grp memb. */
int elem_add_to_grp(struct cn_elem *e, struct cn_grp *g);
int elem_rem_from_grp(struct cn_elem *e, struct cn_grp *g);

/* links */
int elem_link(struct cn_elem *from, struct cn_elem *to);
int elem_unlink(struct cn_elem *from, struct cn_elem *to);

/* getters */
int elem_get_type(struct cn_elem *e);
int elem_get_attr(struct cn_elem *e);
void *elem_get_pdatap(struct cn_elem *e);
void *elem_get_codep(struct cn_elem *e);

int  elem_add_cmd(struct cn_elem *e, struct cn_cmd *cmd);
void *elem_get_cmd(struct cn_elem *e, struct timespec *ts);

//void *elem_get_buf(struct cn_elem *e, struct timespec *ts);

#endif
