
#include<stdlib.h>
#include<stdbool.h>
#include<ctype.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>

#include "util/task_pool.h"


void *worker_task(void *arg)
{
	printf("In worker\n");		
	sleep(2);
        return NULL;		
}

void *worker_task2(void *arg)
{

	printf("In worker2\n");		
	sleep(1);
        return NULL;		
}

int *upper(void *arg, void **res)
{
	char *str = arg;
	char *resp;
	*res = malloc(strlen(str) + 1);	
	resp = *res;

	do {
		*resp = toupper(*str);
		resp++;
	} while(*str++);
   	*resp = '\0';		

	free(res);
	res = NULL;

	/**resu = str2;*/
	return NULL;
}


int main(int argc, char **argv)
{
	int i;
	struct task_pool *tp;

	tp = task_pool_init(40);
	for(i=0; i < 1000000; i++){
		if(task_pool_add(tp, worker_task, NULL)){
			printf("Failed to add work\n");
		}
		usleep(100000);
	}
	task_pool_free(tp);

	return EXIT_SUCCESS;
}

