
#ifndef __CODE_NET_H__
#define __CODE_NET_H__

typedef enum {
   code_t_thread = 0x01,
   code_t_bin = 0x02,
   CODE_NO_INPUT  = 0x04,
} code_t;

typedef struct code_elem code_elem_t;

code_elem_t *code_create(code_t type, void *code, void *pdata);
int code_out_avail(code_elem_t *e, int type, void *buf, int len);
int code_wait(code_elem_t *e);
int code_link(code_elem_t *from, code_elem_t *to);
int code_unlink(code_elem_t *from, code_elem_t *to);

#endif

