#ifndef NN_NODE_H__
#define NN_NODE_H__

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int node_free(struct nn_node *node);

/* router memb. */
int node_add_to_router(struct nn_node *n, struct nn_conn_node_router *cn);
int node_rem_from_router(struct nn_node *n, struct nn_conn_node_router *cn);

/* grp memb. */
int node_add_to_grp(struct nn_node *n, struct nn_grp *g);
int node_rem_from_grp(struct nn_node *n, struct nn_grp *g);

/* links */
int node_link(struct nn_node *from, struct nn_node *to);
int node_unlink(struct nn_node *from, struct nn_node *to);

/* getters */
int node_get_type(struct nn_node *n);
int node_get_attr(struct nn_node *n);
void *node_get_pdatap(struct nn_node *n);
void *node_get_codep(struct nn_node *n);

int  node_add_cmd(struct nn_node *n, struct nn_cmd *cmd);
void *node_get_cmd(struct nn_node *n, struct timespec *ts);

void *node_get_buf(struct nn_node *n, struct timespec *ts);

#endif
