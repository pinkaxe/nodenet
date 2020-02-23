
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>
#include<pthread.h>

#include "async_runner.h"

void *run0(void *arg)
{
	printf("In run0\n");
	sleep(1);
        return NULL;
}

int main(int argc, char **argv)
{
	struct async_runner *tpoolh;

	tpoolh = async_runner_init(5);
	for(;;){
		async_runner_exec(tpoolh, run0, NULL, NULL);
	}
	async_runner_free(tpoolh);

	return 0;
}
