

#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<getopt.h>
#include<pthread.h>

#include "log/log.h"
#include "debug/debug.h"

#include "protocols/protocol_if.h"
#include "protocols/filetrans.h"
#include "sock_serv.h"

#define WORK_THREAD_NUM 100

int main()
{
	struct sproto_if *ph;
	struct server_handle *sh;
    int r;
    int port = 8888;

	ph = serv_proto_ft_init();
	sh = server_create(ph, port, WORK_THREAD_NUM);
	log1(LINFO, "Server started on port: %d", port);
	for(;;){
		sleep(10);
	}
	server_stop(sh);
	ph->free(ph);

err:
	return r;
}
