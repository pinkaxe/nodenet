
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"

#include "node_net/types.h"
#include "node_type.h"

static int node_type_thread_buf_exe(struct nn_node *n, char *buf, size_t len, void *pdata)
{
    return 0; //n->user_func(n, buf, len, pdata);
}


static struct node_type_ops node_type_thread_ops = {
    .node_buf_exe =  node_type_thread_buf_exe
};


struct node_type_ops *node_type_thread_get_ops()
{
    return &node_type_thread_ops;
}
