
#include <stdio.h>

#include "code_net/types.h"

#include "code_net/elem_types/elem_type.h"
#include "code_net/elem_types/thread.h"
//#include "code_net/elem_types/bin.h"
//#include "code_net/elem_types/lproc.h"


/* set the ops depending on type */
struct elem_type_ops *elem_type_get_ops(int type)
{
    struct elem_type_ops *ops = NULL;

    switch(type){
        case CN_TYPE_THREAD:
            ops = elem_type_thread_get_ops();
            break;
       // case CN_TYPE_LPROC:
       //     r = elem_type_lproc_init(e);
       //     break;
       // case CN_TYPE_BIN:
       //     r = elem_type_bin_init(e);
       //     break;
       // case CN_TYPE_SOCK:
       //     break;
    }

    return ops;
}
