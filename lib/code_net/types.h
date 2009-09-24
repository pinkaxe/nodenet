#ifndef __CN_TYPES_H__
#define __CN_TYPES_H__

enum cn_elem_type {
    CN_TYPE_THREAD,
    CN_TYPE_BIN
};

enum cn_elem_attr {
    CN_ATTR_NO_INPUT = 0x01
};

struct cn_net;
struct cn_elem;
struct cn_grp;

#endif
