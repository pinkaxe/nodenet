#ifndef NN_CONN_H__
#define NN_CONN_H__

struct nn_conn_node_router {
    struct nn_node *n;
    struct nn_router *rt;
    mutex_t mutex;

    /* router(output) -> node(input) */
    struct que *rt_n_cmd;   /* router write node cmd */
    struct que *rt_n_data;  /* router write node data */

    /* node(output) -> router(input) */
    struct que *n_rt_notify; /* always used */
    struct que *n_rt_cmd;    /* only master node */
    struct que *n_rt_data;   /* n write data */

    /* internal going to and from router commands */
    struct que *n_rt_int_cmd;
    struct que *rt_n_int_cmd;
};

#endif
