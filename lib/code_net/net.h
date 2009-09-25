
#ifndef __CN_NET_H__
#define __CN_NET_H__

struct cn_net *net_init();
int net_free(struct cn_net *h);

int net_add_memb(struct cn_net *h, struct cn_elem *e);
int net_rem_memb(struct cn_net *h, struct cn_elem *e);
int net_ismemb(struct cn_net *n, struct cn_elem *e);

#endif
