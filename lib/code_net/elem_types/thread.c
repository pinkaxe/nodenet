
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"

#include "code_net/types.h"
#include "elem_type.h"

struct elem_type_ops {
    int (*elem_buf_exe)(struct cn_elem *e, char *buf, size_t len, void *pdata);
    int (*elem_timer_exe)(struct cn_elem *e);
    //int (*elem_cmd_check)(struct cn_elem *e, struct cn_io_cmd **cmd);
    //int (*elem_data_check)(struct cn_elem *e, struct cn_io_data **cmd);
};

static int elem_type_thread_buf_exe(struct cn_elem *e, char *buf, size_t len, void *pdata)
{
    return 0; //e->user_func(e, buf, len, pdata);
}


static struct elem_type_ops elem_type_thread_ops = {
    .elem_buf_exe =  elem_type_thread_buf_exe
};


struct elem_type_ops *elem_type_thread_get_ops()
{
    return &elem_type_thread_ops;
}
