
#include<stdlib.h>
#include<stdbool.h>
#include<ctype.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>

#include "task_pool.h"


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

/*
#define MAP(type, from, to, func) \
int #type#_map(){ \
	while(*fromi){ \
		func(*fromi++, toi++); \
	}		\
	*toi = NULL; \
} while(0); 
*/

/*
int map(void **elems, void **res, int *(*func)(void *arg, void **res))
{
	while(*elems){
		func(*elems++, res++);
	}			
	*res = NULL;
	return 0;
}

#define MAP(from, to, func) \
	map((void **)from, (void **)to, func);

struct test_map1 {
}

int test_map1(void){
*/

int xmain(int argc, char **argv)
{

	/*struct thread_pool *tpoolh;*/
	char *testval[] = {"one", "two bla bla fla", "more bla bla bal", "@!#!@!@ cla", NULL};
	char *testres[sizeof testval], **testrespp;
	/*int res;*/
	
	testrespp = testres;

	/*MAP(char, testval, testres, upper);*/

	/*map((void **)testval, testres, (void **)upper);*/
	

	/*res = map(testval, testres, upper);*/

	while(*testrespp){
		printf("%s\n", *testrespp++);
	}
	return EXIT_SUCCESS;
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

