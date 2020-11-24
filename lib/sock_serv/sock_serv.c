
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>
#include<stdarg.h>

#include "log/log.h"
#include "protocols/protocol_if.h"
#include "sock_serv.h"
#include "debug/debug.h"
#include "file/file.h"
#include "async_runner/async_runner.h"
//#include "thread_pool/thread_pool.h"


struct server_handle {
	int fd;
	int max; /* max connections */
	int port;
	//struct thread_pool *tpoolh;
        struct async_runner *tpoolh;
	struct sproto_if *ph;
};


struct work{
	int fd;		
	struct sproto_if *ph;
};



static void *_server_worker(void *work_)
{
	struct work *work = work_;

	work->ph->do_cmd(work->ph, work->fd);

	log1(LINFO, "Closing server worker: %d", work->fd);
	close(work->fd); 
	free(work);
	work = NULL;
	return NULL;
}

static void _server_conn_res(void *work_, bool succ)
{
	struct work *work = work_;
	if(work->ph->conn_res) {
		work->ph->conn_res(work->ph, work->fd, succ);
	}
}

static void *_server_thread(void *sh_arg)
{
	struct server_handle *sh;
	struct   sockaddr_in pin;
	int 	 cfd;
	socklen_t addrlen;
	struct work *workp;

	sh = sh_arg;

	PTHREAD_TEST(pthread_detach(pthread_self())); 

        addrlen = sizeof(pin); 

	for(;;){
		SOCK_TEST(cfd = accept(sh->fd, 
			(struct sockaddr *)  &pin, &addrlen)); 

		log2(LINFO, "Login from: %s:%d", 
			inet_ntoa(pin.sin_addr),ntohs(pin.sin_port));

		NULL_TEST(workp = malloc(sizeof *workp)); /* free in worker */
		workp->fd = cfd;
		workp->ph = sh->ph;
                if(async_runner_exec(sh->tpoolh, _server_worker, workp, _server_conn_res)) {
		//if(thread_pool_add_work(sh->tpoolh, workp, _server_conn_res)){	
			close(workp->fd); 
			free(workp);
			workp = NULL;
		}
	}
	return NULL;
}


struct server_handle *server_create(struct sproto_if *ph, int port, int max)
{
	struct server_handle *sh;
	struct   sockaddr_in sin;
	pthread_t tid;
	int tr = 1;

 
	NULL_TEST(sh = malloc(sizeof *sh));
	sh->port = port;
	sh->max = max;
	sh->tpoolh = async_runner_init(max);
	sh->ph = ph;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	SOCK_TEST(sh->fd = socket(AF_INET, SOCK_STREAM, 0));
	SOCK_TEST(bind(sh->fd, (struct sockaddr *) &sin, sizeof(sin)));
	SOCK_TEST(setsockopt(sh->fd, SOL_SOCKET,SO_REUSEADDR, &tr, sizeof(int)));
	SOCK_TEST(listen(sh->fd, max) == -1);

	PTHREAD_TEST(pthread_create(&tid, NULL, _server_thread, sh));

	return sh;	
}


int server_stop(struct server_handle *sh)
{
	async_runner_free(sh->tpoolh);
	close(sh->fd);
	free(sh);
	sh = NULL;	
	return 0;
}











