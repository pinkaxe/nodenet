#ifndef NN_NODE_H__
#define NN_NODE_H__

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int node_free(struct nn_node *node);
int node_clean(struct nn_node *n);

/* conn to routers via conns */
int node_conn(struct nn_node *n, struct nn_conn *cn);
int node_unconn(struct nn_node *n, struct nn_conn *cn);

/* grp memb. */
int node_join_chan(struct nn_node *n, struct nn_chan_rel *grp_rel);
int node_quit_chan(struct nn_node *n, struct nn_chan_rel *grp_rel);
struct nn_chan_rel *node_get_chan_rel(struct nn_node *n, struct nn_chan *g);

int node_allow_chan(struct nn_node *n, struct nn_chan *g, int times);

/* getters */
int node_get_type(struct nn_node *n);
int node_get_attr(struct nn_node *n);
void *node_get_pdatap(struct nn_node *n);
void *node_get_codep(struct nn_node *n);
enum nn_state node_get_state(struct nn_node *n);
struct nn_conn *node_get_router_conn(struct nn_node *n, struct nn_router *rt);

/* setters */
int node_set_state(struct nn_node *n, enum nn_state state);

/* node api */
//int node_add_tx_pkt(struct nn_node *n, struct nn_pkt *pkt);
int node_get_rx_pkt(struct nn_node *n, struct nn_pkt **pkt);

int node_set_rx_cnt(struct nn_node *n, int grp_id, int cnt);

int node_tx(struct nn_node *n, struct nn_pkt *pkt);

int node_wait(struct nn_node *n);
int node_do_state(struct nn_node *n);

int node_rx(struct nn_node *n, void **data, int *data_len, void **pdata);

/* number of packets ready for rx */
int node_get_rx_pkts_no(struct nn_node *n);

/* debug/status */

struct node_status {
    //int chan_no;
    //int conn_no;
    int rx_pkts_no; /* how many in que */
    int tx_pkts_no;
    int rx_pkts_total; /* all rx */
    int tx_pkts_total;
};

int node_get_status(struct nn_node *n, struct node_status *status);

#endif
