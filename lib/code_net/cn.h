#ifndef __CN_H__
#define __CN_H__

/* router */
struct cn_router *cn_router_init();
int cn_router_free(struct cn_router *rt);
int cn_router_run(struct cn_router *rt);
int cn_router_set_cmd_cb(struct cn_router *rt, io_cmd_req_cb_t cb);
// cn_router_get_cmd_req(rt, req, timeout)
// cn_router_get_data_req(rt, req, timeout)
int cn_router_add_cmd_req(struct cn_router *rt, struct cn_cmd *cmd);

/* elem */
struct cn_elem *cn_elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata);
int cn_elem_free(struct cn_elem *e);
int cn_elem_run(struct cn_elem *e);
int cn_elem_stop(struct cn_elem *e);

/* grp */
struct cn_grp *cn_grp_init(int id);
int cn_grp_free(struct cn_grp *g);

/* rel */
int cn_add_elem_to_router(struct cn_elem *e, struct cn_router *rt);
int cn_rem_elem_from_router(struct cn_elem *e, struct cn_router *rt);

int cn_add_elem_to_grp(struct cn_elem *e, struct cn_grp *g);
int cn_rem_elem_from_grp(struct cn_elem *e, struct cn_grp *g);

int cn_link_elem(struct cn_elem *from, struct cn_elem *to);
int cn_unlink_elem(struct cn_elem *from, struct cn_elem *to);


#endif
