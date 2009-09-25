
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
#include "elem.h"

#include "dispatchers/thread.h"


int run(struct cn_elem *e)
{
    switch(elem_get_type(e)){
        case CN_TYPE_THREAD:
            dispatcher_thread(e);
            break;
       // case CN_TYPE_SOCK:
       //     dispatcher_sock(e);
       //     break;
    }

    //else if(e->type == CODE_TYPE_BIN){
        // fork here, with pipe to communicate back
   //     thread_t tid;
   //     thread_create(&tid, NULL, code_run_bin, e);
   // }
    //}else if(//network ){
    //  make connection
    //}else if(h->type == FORK_PROCESS){
    //}else if(h->type == REMOTE_PROCESS){
}
