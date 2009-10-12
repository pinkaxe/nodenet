
#include <stdio.h>

#include "node_net/types.h"

#include "node_net/node_types/node_type.h"
#include "node_net/node_types/thread.h"
//#include "node_net/node_types/bin.h"
//#include "node_net/node_types/lproc.h"


/* set the ops depending on type */
struct node_type_ops *node_type_get_ops(int type)
{
    struct node_type_ops *ops = NULL;

    switch(type){
        case NN_NODE_TYPE_THREAD:
            ops = node_type_thread_get_ops();
            break;
       // case NN_NODE_TYPE_LPROC:
       //     r = node_type_lproc_init(n);
       //     break;
       // case NN_NODE_TYPE_BIN:
       //     r = node_type_bin_init(n);
       //     break;
       // case NN_NODE_TYPE_SOCK:
       //     break;
    }

    return ops;
}
