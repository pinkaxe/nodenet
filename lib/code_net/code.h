
#ifndef __CODE_NET_H__
#define __CODE_NET_H__

typedef enum {
   CODE_TYPE_THREAD,
   CODE_TYPE_BIN
} code_type_t;

typedef enum {
    CODE_ATTR_NO_INPUT = 0x01
} code_attr_t;

typedef struct code_elem code_elem_t;

code_elem_t *code_create(code_type_t type, code_attr_t attr, void *code,
        void *pdata);
int code_out_avail(code_elem_t *e, int type, void *buf, int len);
int code_wait(code_elem_t *e);
int code_link(code_elem_t *from, code_elem_t *to);
int code_unlink(code_elem_t *from, code_elem_t *to);

#endif

