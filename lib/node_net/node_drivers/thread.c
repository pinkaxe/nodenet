
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/que.h"

#include "types.h"
#include "pkt.h"
#include "node.h"
#include "node_net/types.h"
#include "node_driver.h"


//static int put_buf(struct nn_node *n, char *buf, size_t len, void *pdata)
//{
//    int (*func)(struct nn_node *n,char *buf, size_t len, void *pdata);
//
//    func = node_get_codep(n);
//
//    return func(n, buf, len, pdata);
//}
//
//static int get_buf(struct nn_node *n, char **buf, size_t len)
//{
//}
//
//static struct node_driver_ops node_driver_thread_ops = {
//    .put_buf = put_buf,
//    .get_buf = get_buf
//};
//
//struct node_driver_ops *node_driver_thread_get_ops()
//{
//    return &node_driver_thread_ops;
//}
