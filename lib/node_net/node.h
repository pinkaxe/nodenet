#ifndef NN_NODE_H__
#define NN_NODE_H__


enum wait_type {
    NN_RX_READY,
    NN_TX_READY,
    NN_RXTX_READY
};

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata);
int node_free(struct nn_node *n);

/* connections  */

int node_conn(struct nn_node *n, struct nn_conn *cn);
int node_unconn(struct nn_node *n, struct nn_conn *cn);

/* node driver api */

void *node_get_pdatap(struct nn_node *n);

void node_block_rx(struct nn_node *n, bool block);

int node_tx(struct nn_node *n, struct nn_pkt *pkt);
int node_rx(struct nn_node *n, struct nn_pkt **pkt);
int node_wait(struct nn_node *n, enum wait_type type);
int node_do_state(struct nn_node *n);

int node_put_pkt(struct nn_node *n, struct nn_pkt *pkt);
int node_get_pkt(struct nn_node *n, struct nn_pkt **pkt);

//int node_join_chan(struct nn_node *n, struct nn_chan_rel *grp_rel);
//int node_quit_chan(struct nn_node *n, struct nn_chan_rel *grp_rel);
//struct nn_chan_rel *node_get_chan_rel(struct nn_node *n, struct nn_chan *g);


/* getters */
//int node_get_attr(struct nn_node *n);
//static void *node_get_codep(struct nn_node *n);
enum nn_state node_get_state(struct nn_node *n);
struct nn_conn *node_get_router_conn(struct nn_node *n, struct nn_router *rt);

/* main */
/* setters */
int node_set_state(struct nn_node *n, enum nn_state state);


int node_set_rx_cnt(struct nn_node *n, int grp_id, int cnt);


/* number of packets ready for rx */
int node_rxs_no(struct nn_node *n);

/* status */

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
