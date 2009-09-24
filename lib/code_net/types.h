#ifndef __CN_TYPES_H__
#define __CN_TYPES_H__

enum cn_elem_type {
    CN_TYPE_THREAD,
    CN_TYPE_BIN
};

enum cn_elem_attr {
    CN_ATTR_NO_INPUT = 0x01
};

enum cn_elem_cmd {
    CN_ELEM_CMD_STOP
};

struct cn_net;
struct cn_elem;
struct cn_grp;
struct cn_grp_memb;

#endif
