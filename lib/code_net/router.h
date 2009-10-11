
#ifndef __cn_router_H__
#define __cn_router_H__

struct cn_router *router_init();
int router_free(struct cn_router *h);

int router_run(struct cn_router *h);

int router_add_memb(struct cn_router *h, struct cn_elem *e);
int router_rem_memb(struct cn_router *h, struct cn_elem *e);
int router_ismemb(struct cn_router *rt, struct cn_elem *e);
struct cn_elem *router_memb_iter(struct cn_router *rt, void **iter);

int router_set_cmd_cb(struct cn_router *rt, io_cmd_req_cb_t cb);

int router_add_cmd(struct cn_router *rt, struct cn_cmd *cmd);
struct cn_cmd *router_get_cmd(struct cn_router *rt, struct timespec *ts);


#endif
