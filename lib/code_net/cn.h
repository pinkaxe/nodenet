#ifndef __CN_H__
#define __CN_H__

/* net */
struct cn_net *cn_net_init();
int cn_net_free(struct cn_net *n);
//cn_net_run
int cn_net_set_cmd_cb(struct cn_net *n, io_cmd_req_cb_t cb);
// cn_net_get_cmd_req(n, req, timeout)
// cn_net_get_data_req(n, req, timeout)
int cn_net_add_cmd_req(struct cn_net *n, struct cn_io_cmd_req *req);

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
int cn_add_elem_to_net(struct cn_elem *e, struct cn_net *n);
int cn_rem_elem_from_net(struct cn_elem *e, struct cn_net *n);

int cn_add_elem_to_grp(struct cn_elem *e, struct cn_grp *g);
int cn_rem_elem_from_grp(struct cn_elem *e, struct cn_grp *g);

int cn_link_elem(struct cn_elem *from, struct cn_elem *to);
int cn_unlink_elem(struct cn_elem *from, struct cn_elem *to);


#endif
