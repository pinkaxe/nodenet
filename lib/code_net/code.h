
#ifndef __CODE_NET_H__
#define __CODE_NET_H__


typedef enum {
   code_t_thread,
   code_t_bin,
} code_t;

struct code_elem;

int code_out_avail(struct code_elem *e, int type, unsigned char *buf, int len); // TYPE = all, one, specific ones
int code_wait(struct code_elem *e);
int code_link(struct code_elem *e0, struct code_elem *e1);
int code_unlink(struct code_elem *e0, struct code_elem *e1);

#endif

