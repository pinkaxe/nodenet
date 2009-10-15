#ifndef NN_NODE_H__
#define NN_NODE_H__

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int node_free(struct nn_node *node);


/* conn to routers via conns */
int node_conn(struct nn_node *n, struct nn_conn *cn);
int node_unconn(struct nn_node *n, struct nn_conn *cn);
struct nn_conn *node_conn_iter(struct nn_node *n, void **iter);

/* grp memb. */
int node_join_grp(struct nn_node *n, struct nn_grp *g);
int node_quit_grp(struct nn_node *n, struct nn_grp *g);

/* getters */
int node_get_type(struct nn_node *n);
int node_get_attr(struct nn_node *n);
void *node_get_pdatap(struct nn_node *n);
void *node_get_codep(struct nn_node *n);

#endif
