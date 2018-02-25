
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>
#include<pthread.h>

#include "dlog.h"
#include "debug.h"
#include "protocols/protocol_if.h"
#include "protocols/filetrans.h"
#include "sock_serv.h"

#define WORK_THREAD_NUM	5

static void usage(char *name)
{
	puts("\nUsage:");
	printf("%s port\n\n", name);
	exit(1);
}

static void signal_pipe(int signo)
{
	DLOG0(LOG_WARNING, "Caught signal pipe");	
}


int main(int argc, char **argv)
{
	struct sproto_if *ph;
	struct server_handle *sh;

	int port;
	if(argc < 2){
		usage(*argv);
	}
	if(!(port = atoi(argv[1]))){
		usage(*argv);
	}		

	if(signal(SIGPIPE, signal_pipe) == SIG_ERR){
		DLOG1(LOG_ERR, "Couldn't create sighandler: %d", SIGPIPE);	
	}

	ph = serv_proto_ft_init();
	sh = server_create(ph, port, WORK_THREAD_NUM);
	DLOG1(LOG_INFO, "Server started on port: %d", port);
	for(;;){
		sleep(10);
	}
	server_stop(sh);
	ph->free(ph);

	return 0;
}
