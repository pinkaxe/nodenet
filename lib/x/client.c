
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>
#include<pthread.h>
#include<signal.h>


#include "dlog.h"
#include "debug.h"
#include "protocols/protocol_if.h"
#include "protocols/filetrans.h"
#include "sock_cli.h"

#define PORT	9999

static void usage(char *name)
{
	puts("\nUsage:");
	printf("%s host port cmd file\n\n", name);
	exit(1);
}

static void signal_pipe(int signo)
{
	DLOG0(LOG_WARNING, "Caught signal pipe");	
}

int main(int argc, char **argv)
{
	struct cproto_if *ph;
	struct client_handle *ch;

	char *host;
	int port;		
	char *cmd;
	char *file;

	if(argc != 5){
		usage(*argv);
	}

	host = argv[1];
	port = atoi(argv[2]);
	cmd = argv[3];
	file = argv[4];

	printf("host: %s\n", host);
	printf("port: %d\n", port);
	printf("cmd: %s\n", cmd);
	printf("file: %s\n", file);
	puts("");
	

	if(signal(SIGPIPE, signal_pipe) == SIG_ERR){
		DLOG1(LOG_ERR, "Couldn't create sighandler: %d", SIGPIPE);	
	}

	ph = cli_proto_ft_init();
	if((ch = client_create(ph, host, port))){
		printf("Result: %s\n\n\n", 
			ph->get_proto_error(ph, client_do(ch, cmd, file)));
		DLOG0(LOG_DEBUG, "Stopping client\n");
		client_stop(ch);
	}else{
		DLOG0(LOG_ERR, "Couldn't connect\n");
	}
	ph->free(ph);

	return 0;
}


