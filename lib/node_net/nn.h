#ifndef NN_NN_H__
#define NN_NN_H__

/* router */
struct nn_router *nn_router_init();
int nn_router_free(struct nn_router *rt);
int nn_router_clean(struct nn_router *rt);
int nn_router_set_state(struct nn_router *rt, enum nn_state state);
int nn_router_print(struct nn_router *rt);

int nn_router_set_pkt_cb(struct nn_router *rt, io_pkt_req_cb_t cb);
int nn_node_set_state(struct nn_node *n, enum nn_state state);
// nn_router_get_pkt_req(rt, req, timeout)
// nn_router_get_data_req(rt, req, timeout)
int nn_router_tx_pkt(struct nn_router *rt, struct nn_node *n, struct nn_pkt
        *pkt);
int nn_node_tx_pkt(struct nn_node *n, struct nn_router *rt, struct nn_pkt
        *pkt);

/* node */
struct nn_node *nn_node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int nn_node_free(struct nn_node *n);
int nn_node_clean(struct nn_node *n);
int nn_node_run(struct nn_node *n);
int nn_node_stop(struct nn_node *n);
int nn_node_print(struct nn_node *n);

/* grp */
struct nn_grp *nn_grp_init(int id);
int nn_grp_free(struct nn_grp *g);

/* rel */
int nn_conn(struct nn_node *n, struct nn_router *rt);
int nn_unconn(struct nn_node *n, struct nn_router *rt);

int nn_join_grp(struct nn_node *n, struct nn_grp *g);
int nn_quit_grp(struct nn_node *n, struct nn_grp *g);


int nn_node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt);

#endif
