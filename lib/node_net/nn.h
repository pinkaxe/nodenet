#ifndef __nn_H__
#define __nn_H__

/* router */
struct nn_router *nn_router_init();
int nn_router_free(struct nn_router *rt);
int nn_router_run(struct nn_router *rt);
int nn_router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb);
// nn_router_get_cmd_req(rt, req, timeout)
// nn_router_get_data_req(rt, req, timeout)
int nn_router_add_cmd_req(struct nn_router *rt, struct nn_cmd *cmd);

/* node */
struct nn_node *nn_node_init(enum nn_node_type type, enum nn_node_attr attr,
        void *code, void *pdata);
int nn_node_free(struct nn_node *n);
int nn_node_run(struct nn_node *n);
int nn_node_stop(struct nn_node *n);

/* grp */
struct nn_grp *nn_grp_init(int id);
int nn_grp_free(struct nn_grp *g);

/* rel */
int nn_add_node_to_router(struct nn_node *n, struct nn_router *rt);
int nn_rem_node_from_router(struct nn_node *n, struct nn_router *rt);

int nn_add_node_to_grp(struct nn_node *n, struct nn_grp *g);
int nn_rem_node_from_grp(struct nn_node *n, struct nn_grp *g);

int nn_link_node(struct nn_node *from, struct nn_node *to);
int nn_unlink_node(struct nn_node *from, struct nn_node *to);


#endif
