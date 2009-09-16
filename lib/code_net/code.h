
#ifndef __CODE_NET_H__
#define __CODE_NET_H__


typedef enum {
   code_t_thread = 0x01,
   code_t_bin = 0x02,
   CODE_NO_INPUT  = 0x04,
   CODE_NO_OUTPUT = 0x08,
} code_t;

struct code_elem;

struct code_elem *code_create(code_t type, void *code, void *pdata);
int code_out_avail(struct code_elem *e, int type, void *buf, int len); // TYPE = all, one, specific ones
int code_wait(struct code_elem *e);
int code_link(struct code_elem *e0, struct code_elem *e1);
int code_unlink(struct code_elem *e0, struct code_elem *e1);

#endif

