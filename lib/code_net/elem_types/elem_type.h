#ifndef ELEM_TYPE_THREAD_H__
#define ELEM_TYPE_THREAD_H__

struct elem_type_ops {
    int (*elem_buf_exe)(struct cn_elem *e, char *buf, size_t len, void *pdata);
    int (*elem_timer_exe)(struct cn_elem *e);
    //int (*elem_cmd_check)(struct cn_elem *e, struct cn_io_cmd **cmd);
    //int (*elem_data_check)(struct cn_elem *e, struct cn_io_data **cmd);
};


struct elem_type_ops *elem_type_get_ops(int type);

#endif
