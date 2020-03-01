
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
	printf("In run0: %d\n", *(int *)arg);
    //sleep(1);
    return NULL;
}

void run0_res(void *arg, bool res)
{
	printf("In run0_res: %d %d\n", res, *(int *)arg);
}

int main(int argc, char **argv)
{
	struct async_runner *tpoolh;

	tpoolh = async_runner_init(5);
        //for(;;){
        int arg0 = 33;
        int arg1 = 66;
        async_runner_exec(tpoolh, run0, &arg0, run0_res);
        async_runner_exec(tpoolh, run0, &arg1, run0_res);
        sleep(2);
	//}
	async_runner_free(tpoolh);

	return 0;
}
