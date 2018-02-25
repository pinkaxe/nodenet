
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>
#include<pthread.h>

#include "../thread_pool.h"



void *worker_thread(void *arg)
{

	printf("In worker\n");		
	sleep(1);
        return NULL;		
}

int main(int argc, char **argv)
{
	struct thread_pool *tpoolh;

	tpoolh = thread_pool_init(5, worker_thread);
	for(;;){
		thread_pool_add_work(tpoolh, NULL, NULL);
	}
	thread_pool_free(tpoolh);

	return 0;
}
