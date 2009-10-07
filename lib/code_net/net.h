
#ifndef __CN_NET_H__
#define __CN_NET_H__

struct cn_net *net_init();
int net_free(struct cn_net *h);

int net_run(struct cn_net *h);
int net_add_memb(struct cn_net *h, struct cn_elem *e);
int net_rem_memb(struct cn_net *h, struct cn_elem *e);
int net_ismemb(struct cn_net *n, struct cn_elem *e);

int net_set_cmd_cb(struct cn_net *n, io_cmd_req_cb_t cb);
int net_add_cmd_req(struct cn_net *n, struct cn_io_cmd *cmd);

int net_set_data_cb(struct cn_net *n, io_data_req_cb_t cb);
int net_add_data_req(struct cn_net *n, struct cn_io_data *data);

int net_sendto_all(struct cn_net *n, struct cn_io_cmd *cmd);

#endif
