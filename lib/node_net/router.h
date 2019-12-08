
#ifndef NN_ROUTER_H__
#define NN_ROUTER_H__

struct nn_router *router_init();
int router_free(struct nn_router *rt);

int router_add_chan(struct nn_router *rt, struct nn_chan *chan);
/* return -1 no such group */
int router_rem_chan(struct nn_router *rt, int id);
struct nn_chan *router_get_chan(struct nn_router *rt, int id);

int router_add_node(struct nn_router *rt, struct nn_node *n);
int router_rem_node(struct nn_router *rt, struct nn_node *n);

//int router_conn(struct nn_router *rt, struct nn_conn *cn);
//int router_unconn(struct nn_router *rt, struct nn_conn *n);
//int router_isconn(struct nn_router *rt, struct nn_node *n);

enum nn_state router_get_state(struct nn_router *rt);
int router_set_state(struct nn_router *rt, enum nn_state state);

int router_print(struct nn_router *rt);

int router_add_to_chan(struct nn_router *rt, struct nn_chan *ch, struct nn_node *n);
int router_rem_from_chan(struct nn_router *rt, int grp_id, struct nn_node *n);

/* notify router that conn buf avail */
int router_conn_buf_avail(struct nn_router *rt, int cnt);

/* debug/status */

struct router_status {
    //int chan_no;
    //int conn_no;
    int rx_pkts_no; /* how many in que */
    int tx_pkts_no;
    int rx_pkts_total; /* all rx */
    int tx_pkts_total;
};

/* status must be allocated */
int router_get_status(struct nn_router *rt, struct router_status *status);

struct nn_chan *chan_init(void);
int chan_free(struct nn_chan *chan);

#endif
