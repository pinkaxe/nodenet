
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"
#include "sys/thread.h"

#include "code_net/types.h"
#include "code_net/elem_types/elem_type.h"


int thread_elem_type_user_func(struct cn_elem *e, char *buf, size_t len, void *pdata)
{
    return user_func(e, buf, len, pdata);
}

static struct elem_ops {
    elem_exe : elem_type_thread_exe;
} elem_type_thread_ops;


int elem_type_thread_init(struct cn_elem *e) 
{
    return elem_type_thread_ops;
}
