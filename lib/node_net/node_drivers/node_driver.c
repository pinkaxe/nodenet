
#include <stdio.h>

#include "node_net/types.h"

#include "node_net/node_drivers/node_driver.h"
#include "node_net/node_drivers/thread.h"

/* set the ops depending on type */
struct node_driver_ops *node_driver_get_ops(int type)
{
    struct node_driver_ops *ops = NULL;

    switch(type){
        case NN_NODE_TYPE_THREAD:
            ops = node_driver_thread_get_ops();
            break;
       // case NN_NODE_DRIVER__LPROC:
       //     r = node_driver_lproc_init(n);
       //     break;
       // case NN_NODE_DRIVER__BIN:
       //     r = node_driver_bin_init(n);
       //     break;
       // case NN_NODE_DRIVER__SOCK:
       //     break;
    }

    return ops;
}
