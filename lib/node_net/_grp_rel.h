#ifndef _NN_GRP_REL_H__
#define _NN_GRP_REL_H__

struct nn_grp_rel *_grp_rel_init();
int _grp_rel_free(struct nn_grp_rel *grp_rel);
int _grp_rel_free_node(struct nn_grp_rel *grp_rel);
int _grp_rel_free_grp(struct nn_grp_rel *grp_rel);

int _grp_rel_set_node(struct nn_grp_rel *grp_rel, struct nn_node *n);
int _grp_rel_set_grp(struct nn_grp_rel *grp_rel, struct nn_grp *rt);
//int grp_rel_set_state(struct nn_grp_rel *grp_rel, enum nn_grp_rel_state state);

int _grp_rel_set_times(struct nn_grp_rel *grp_rel, int times);
int _grp_rel_dec_times(struct nn_grp_rel *grp_rel, int dec);

struct nn_node *_grp_rel_get_node(struct nn_grp_rel *grp_rel);
struct nn_grp *_grp_rel_get_grp(struct nn_grp_rel *grp_rel);
int _grp_rel_get_state(struct nn_grp_rel *grp_rel);

/* grp_relections between grps and nodes */
int _grp_rel_grp_rel(struct nn_node *n, struct nn_grp *rt);
int _grp_rel_ungrp_rel(struct nn_node *n, struct nn_grp *rt);

/* grp -> node pkt */
int _grp_rel_node_rx_pkt(struct nn_grp_rel *grp_rel, struct nn_pkt **pkt);
int _grp_rel_node_tx_pkt(struct nn_grp_rel *grp_rel, struct nn_pkt *pkt);
int _grp_rel_grp_tx_pkt(struct nn_grp_rel *grp_rel, struct nn_pkt *pkt);
int _grp_rel_grp_rx_pkt(struct nn_grp_rel *grp_rel, struct nn_pkt **pkt);

int _grp_rel_lock(struct nn_grp_rel *grp_rel);
int _grp_rel_unlock(struct nn_grp_rel *grp_rel);

#endif
