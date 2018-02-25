#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
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

#include "dlog.h"
#include "protocols/protocol_if.h"
#include "sock_cli.h"
#include "debug.h"
#include "file.h"


struct client_handle {
	int fd;
	struct cproto_if *ph;
};

struct work{
	int fd;		
	const char *cmd;
	void *data;
	struct cproto_if *ph;
};


static int _client_worker(void *work_)
{
	int ret;
	struct work *work = work_;
	
	ret = work->ph->do_cmd(work->ph, work->fd, work->cmd, work->data);
	free(work);
	work = NULL;

	return ret;
}


struct client_handle *client_create(struct cproto_if *ph, char *host, int port)
{
	struct client_handle *ch;
	struct sockaddr_in sin;
	struct hostent *hostp;

	NULL_TEST(ch = malloc(sizeof *ch));

	if(!(hostp=gethostbyname(host))){
		NULL_TEST(hostp = 
			gethostbyaddr(host, strlen(host), AF_INET));
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	memcpy(&sin.sin_addr, hostp->h_addr_list[0], hostp->h_length);

	ch->ph = ph;
	SOCK_TEST(ch->fd = socket(AF_INET, SOCK_STREAM, 0));
	DLOG1(LOG_INFO, "Connecting client worker: %d", ch->fd);
	SOCK_TEST(connect(ch->fd, (struct sockaddr*) &sin, sizeof(sin)));
	
	return ch;
}

int client_do(struct client_handle *ch, const char *cmd, void *data)
{
	struct work *workp;

	ASSERT(ch);

	NULL_TEST(workp = malloc(sizeof *workp)); /* free in worker */
	workp->fd = ch->fd;
	workp->cmd = cmd;
	workp->data = data;
	workp->ph = ch->ph;
	return _client_worker(workp);
}

int client_stop(struct client_handle *ch)
{

	ASSERT(ch);

	DLOG1(LOG_INFO, "Closing client: %d", ch->fd);
	close(ch->fd);
	free(ch);
	ch = NULL;	
	return 0;
}
