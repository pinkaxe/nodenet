

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <sys/socket.h>

#include "util/log.h"
#include "arch/thread.h"
#include "code_net/types.h"

/*
int dispatcher_bin(struct cn_elem *e) 
{
    thread_t tid;
    thread_create(&tid, NULL, thread_loop, e);
}
*/

static int fd[2];


int bin_loop()
{
    char buf[5];
    read(fd[1], buf, 4);
    printf("!! read0: %s\n", buf);
    write(fd[1], "no", 3);
    while(1){
        // wait for input to element, fork, exe and 
        sleep(1);
        printf("in process\n");
    }
end:
    printf("... bin exit\n");
    return 0;

}


int dispatcher_bin(struct cn_elem *e)
{
    int r = 0;
    int pid;

    // create stream pipe
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

   // pid = fork();
   // if(pid < 0){
   //     r = 1;
   //     goto err;
   // }else if(pid > 0){
   //     // parent
   //     close(fd[1]);
   //     //printf("parent\n");
   //     thread_t tid;
   //     thread_create(&tid, NULL, bin_middle, e);
   // }else if(pid == 0){
   //     // child
   //     close(fd[0]);

      //  if(fd[1] != STDIN_FILENO)
      //      dup2(fd[1], STDIN_FILENO);

      //  if(fd[1] != STDOUT_FILENO)
      //      dup2(fd[1], STDOUT_FILENO);

   //     //printf("child\n");
   //     bin_loop();
   // }

err:
    return r;
}
