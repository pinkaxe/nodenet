
#ifndef __CODE_NET_H__
#define __CODE_NET_H__

typedef enum {
    CODE_TYPE_THREAD,
    CODE_TYPE_BIN
} code_type_t;

typedef enum {
    CODE_ATTR_NO_INPUT = 0x01
} code_attr_t;

typedef enum {
    CODE_CMD_PAUSE,
    CODE_CMD_CONTINUE,
    CODE_CMD_STOP,
} code_cmd_t;

typedef enum {
    BUF_ATTR_RO = 0x01,
    BUF_ATTR_RW = 0x02,
    BUF_ATTR_SEND_ALL = 0x04,
    BUF_ATTR_SEND_ONE = 0x08,
    BUF_ATTR_CLEANUP = 0x04
} buf_attr_t; // send_type_t

typedef struct code_elem code_elem_t;

code_elem_t *code_create(code_type_t type, code_attr_t attr, void *code,
        void *pdata);
// code_clone

int code_link(code_elem_t *from, code_elem_t *to);
int code_unlink(code_elem_t *from, code_elem_t *to);

int code_out_avail(code_elem_t *e, buf_attr_t attr, void *buf, int len, void
        (*sending_to_no_cb)(void *buf, int no));

//int code_sendto_all(code_elem_t *e, void *buf, int len);
//int code_sendto_nr(code_elem_t *e, int no, code_elem_t **to, void *buf, int
//        len);
//int code_sendto_grp(code_elem_t *e, grp_t grp, void *buf, int len);
//
//int code_get_conn_grps(code_elem_t *e, grp_t grp, void *buf, int len);

#endif

