
/* protocol handler
 *
 * first just after connecting the client received CONN_OK(uint8_t)
 *
 * after that commands can be sent as following
 * from first byte to send and receive
 * uint8_t - type specifies the type, only 1 for now
 * uint16_t - cmd look in struct cmd_func_map for possible commands,
 * uint32_t - length of first data field
 * char []  - first data field
 * uint32_t - length of second data field
 * char []  - second data field
 *
 * and so on depending on the command,
 *
 * PUT would be sent like like:
 * \x01 \x00 \x01
 * And then the length of the filename and the filename
 * \x00 \x00 \x00 \x07 f i l e . p n g 
 * And then the length of the file data and the file data
 * \x00 \x00 \x10 \x00 ..........................
 */


#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<assert.h>
#include<sys/select.h>


#include "file/file.h"
#include "debug/debug.h"
#include "log/log.h"
#include "protocol_if.h"
#include "filetrans.h"

#define WRITE_BUF_SIZE	128
#define READ_BUF_SIZE	256
#define MAX_FILE_LEN	128   /* FIXME: make dynamic */
#define RW_TIMEOUT_SEC	5


enum cmds {
	PUT = 0x01
};

enum return_codes {
	RET_OK = 0x00,
	RET_NO_CONN_AVAIL,
	RET_ERR,
	RET_READ_ERR,
	RET_WRITE_ERR,
	RET_TIMEOUT,
	RET_FILEEXIST,
	RET_FILENAMEERR,
	RET_COULDNTOPENFILE,
	RET_CMD_NOT_FOUND
};

static struct return_mesg {
	int code;
	char *mesg;
} _return_mesg[] = {	
	{RET_OK, "RET_OK"},
	{RET_NO_CONN_AVAIL, "RET_NO_CONN_AVAIL"},
	{RET_ERR, "RET_ERR"},
	{RET_READ_ERR, "RET_READ_ERR"},
	{RET_WRITE_ERR, "RET_WRITE_ERR"},
	{RET_TIMEOUT, "RET_TIMEOUT"},
	{RET_FILEEXIST, "RET_FILEEXIST"},
	{RET_FILENAMEERR, "RET_FILENAMEERR"},
	{RET_COULDNTOPENFILE, "RET_COULDNTOPENFILE"},
	{RET_CMD_NOT_FOUND, "RET_CMD_NOT_FOUND"},
};

static int _return_mesg_no = sizeof(_return_mesg) / sizeof(_return_mesg[0]);

static int write_to(int fd, struct timeval *timeout, 
		const void *buf, int size, size_t *ret_no);
static int read_to(int fd, struct timeval *timeout, 
		void *buf, int size, size_t *ret_no);

static int do_serv_cmd(struct sproto_if *self, int fd);
static void serv_proto_ft_free(struct sproto_if *self);
static int serv_put(int fd);
static const char *get_serv_proto_error(struct sproto_if *self, 
		int error_no);
static void serv_conn_res(struct sproto_if *self, int fd, bool succ);

static int do_cli_cmd(struct cproto_if *self, int fd, 
		const char *cmd, void *data);
static void cli_proto_ft_free(struct cproto_if *self);
static int cli_put(int fd, void *data);
static const char *get_cli_proto_error(struct cproto_if *self, 
		int error_no);

static const char *get_proto_error(int error_no);


/* commands for client/server */
static struct cmd_func_map {
	uint16_t cmd;
	char *cmd_str;
	int (*cli_func)(int fd, void *data);
	int (*serv_func)(int fd);
} _cmd_func_map[] = {
	{ 		
		PUT, "PUT", cli_put, serv_put
	},
};

static int _cmd_func_map_no = sizeof(_cmd_func_map) / sizeof(_cmd_func_map[0]);




/* server */


struct sproto_if *serv_proto_ft_init(void)
{
	struct sproto_if *self;
	NULL_TEST(self = malloc(sizeof *self));

	self->do_cmd = do_serv_cmd;
	self->get_proto_error = get_serv_proto_error;
	self->conn_res = serv_conn_res;
	self->free = serv_proto_ft_free;

	return self;
}

static void serv_proto_ft_free(struct sproto_if *self)
{
	free(self);
	self = NULL;
}


static void serv_conn_res(struct sproto_if *self, int fd, bool succ){
	uint8_t conn_resp;
	size_t w;

	if(succ){
		conn_resp = RET_OK;
	}else{
		conn_resp = RET_NO_CONN_AVAIL;
	}
	write_to(fd, NULL, &conn_resp, sizeof(conn_resp), &w);
}

/* function to handle incoming files */
static int serv_put(int fd)
{
	char 	 buf[READ_BUF_SIZE];
	int      ofd; /* output */
	uint32_t lenn; /* network byte order */
	uint32_t len; 
	size_t 	 r, w;
	char 	 filename[MAX_FILE_LEN];
	char 	 save_as[MAX_FILE_LEN+9] = "uploaded/"; /* FIXME:make configurable */
	int 	 ret = RET_OK;


	if((ret=read_to(fd, NULL, &lenn, sizeof(lenn), &r)) != RET_OK){
		return ret;
	}

	len = ntohl(lenn);
	log1(LINFO, "Length of filename of incoming file: %d", len);
	
	if(len > MAX_FILE_LEN){
		return RET_ERR;
	}

	if((ret=read_to(fd, NULL, filename, len, &r)) != RET_OK){
		return ret;
	}

	filename[r] = '\0';
	log1(LINFO, "Filename of incoming file: %s", filename);

	if((ret=read_to(fd, NULL, &lenn, sizeof(lenn), &r)) != RET_OK){
		return ret;
	}

	len = ntohl(lenn);
	log1(LINFO, "Length of incoming file: %d", len);


	strncat(save_as, filename, MAX_FILE_LEN);
	if((ofd=open(save_as, O_WRONLY | O_CREAT | O_EXCL)) == -1){
		if(errno == EEXIST){
			log2(LWARN, "File already exist: %s(%s)", 
				save_as, strerror(errno));
			return RET_FILEEXIST;
		}else{
			log2(LWARN, "Couldn't open file: %s(%s)", 
				save_as, strerror(errno));
			return RET_ERR;
		}
		while(len > 0) {
			if((ret=read_to(fd, NULL, &buf, 
					READ_BUF_SIZE, &r)) != RET_OK){
				break;
			}
			len -= r;
		}
		return RET_ERR;
	}else{
		while(len > 0) {
			if((ret=read_to(fd, NULL, &buf, 
					READ_BUF_SIZE, &r)) != RET_OK){
				goto close_file;
			}

			if((w=write(ofd, buf, r)) == -1) {
				log2(LWARN, "Couldn't write file: %s(%s)", 
					save_as, strerror(errno));
				ret = RET_ERR;
				goto close_file;
			}
			len -= r;
		}

		if(chmod(save_as, 0666) == -1){
			log2(LWARN, "Couldn't chmod file: %s(%s)", 
				save_as, strerror(errno));
			ret = RET_ERR;
		}
		
close_file:
		if(close(ofd) == -1){
			log2(LWARN, "Couldn't close file: %s(%s)", 
				save_as, strerror(errno));
			ret = RET_ERR;
		}
	}

	if(ret == RET_OK){
		log1(LINFO, "Successfully saved file: %s", save_as);
	}

	return ret;
}




static int do_serv_cmd(struct sproto_if *self, int fd)
{
	/* ignore type for now */	
	uint8_t	 type;
	uint16_t cmdn; /* network byte order */
	uint16_t cmd;
	size_t	r, w;
	int	i;
        uint8_t ret = RET_ERR;	

	if((ret=read_to(fd, NULL, &type, sizeof(type), &r)) != RET_OK){
		return ret;
	}
	if((ret=read_to(fd, NULL, &cmdn, sizeof(cmdn), &r)) != RET_OK){
		return ret;
	}
		
	cmd = ntohs(cmdn);
	log1(LINFO, "Got command number: %d", cmd);

	for(i=0; i < _cmd_func_map_no; i++){	
		if(cmd == _cmd_func_map[i].cmd){
			log1(LINFO, "Found command to execute: %s", 
				_cmd_func_map[i].cmd_str);
			ret = _cmd_func_map[i].serv_func(fd);
			break;
		}
	}

	write_to(fd, NULL, &ret, sizeof(ret), &w);

	return ret;
}

static const char *get_serv_proto_error(struct sproto_if *self, int error_no)
{
	return get_proto_error(error_no);
}


/* client */

struct cproto_if *cli_proto_ft_init(void)
{
	struct cproto_if *self;
	NULL_TEST(self = malloc(sizeof *self));

	self->do_cmd = do_cli_cmd;
	self->get_proto_error = get_cli_proto_error;
	self->free = cli_proto_ft_free;

	return self;
}

static void cli_proto_ft_free(struct cproto_if *self)
{
	free(self);
	self = NULL;
}

static int cli_put(int fd, void *arg)
{
	char *filename = arg;
	char *save_as;
	uint32_t filenamelen;
	uint32_t filenamelenn; /* network byte order */ 
	uint32_t filelen;
	uint32_t filelenn;   
	char buf[WRITE_BUF_SIZE];
	int ffd;
	size_t r, w;
	int ret = RET_OK;

	if(!(save_as=strrchr(filename, '/'))){
		save_as = filename;
	}else{
		save_as++;
	}

	filenamelen = strlen(save_as);
	printf("save_as: %s\n", save_as);

	if((filelen = file_len(filename))){
		filenamelenn = htonl(filenamelen);
		filelenn = htonl(filelen);

		if((ffd = open(filename, O_RDONLY)) == -1){
			log2(LERR, "Couldn't open file: %s(%s)", 
				filename, strerror(errno));
			return RET_COULDNTOPENFILE;
		}

		if((ret=write_to(fd, NULL, &filenamelenn, 
				sizeof(filenamelenn), &w)) != RET_OK){
			goto close;
		}

		if((ret=write_to(fd, NULL, save_as, 
				strlen(save_as), &w)) != RET_OK){
			goto close;
		}

		/* file */			
		if((ret=write_to(fd, NULL, &filelenn, 
				sizeof(filelenn), &w)) != RET_OK){
			goto close;
		}

		while(filelen > 0){
			r = read(ffd, &buf, WRITE_BUF_SIZE);
			if(r == -1){
				log1(LWARN, "Couldn't read file: %s", 
					strerror(errno));
				ret = RET_ERR;
				goto close;
			}
			if((ret=write_to(fd, NULL, &buf, r, &w)) != RET_OK){
				goto close;
			}

			filelen -= r;	
		}		
		if(filelen == 0) ret = RET_OK;	

close:
		if(close(ffd) != 0){
			log1(LWARN, "Couldn't close file: %s", 
				strerror(errno));
			ret = RET_ERR;
		}
	}else{
		log3(LWARN, "Couldn't open file: %s(%s), %d", 
			filename, strerror(errno), errno);

		/*if(errno == ENOENT){*/ /* IMPROVE: more err messages */
		ret = RET_COULDNTOPENFILE;
		/*}*/
	}

	return ret;

}

static int do_cli_cmd(struct cproto_if *self, int fd, const char *cmd, void *data)
{
	uint8_t  type = 0x01;
	uint16_t cmdn;   
	int 	i;
	size_t  w, r;
        uint8_t ret = RET_ERR;	
	uint8_t con_resp;

	/* connect resp test */
	if((ret=read_to(fd, NULL, &con_resp, 1, &r)) != RET_OK){
		return ret;
	}
	if(con_resp == RET_NO_CONN_AVAIL){
		return con_resp;
	}

	for(i=0; i < _cmd_func_map_no; i++){	
		if(!strcmp(cmd, _cmd_func_map[i].cmd_str)){
			log1(LINFO, "Found command to execute: %s", cmd);
			cmdn = htons(_cmd_func_map[i].cmd);
			/* header */
			if((ret=write_to(fd, NULL, &type, 
					sizeof(type), &w)) != RET_OK){
				break;
			}
			if((ret=write_to(fd, NULL, &cmdn, 	
					sizeof(cmdn), &w)) != RET_OK){
				break;
			}

			ret = _cmd_func_map[i].cli_func(fd, data);
			if(ret == RET_OK || ret == RET_ERR || 
					ret == RET_WRITE_ERR){		
				if((r=read(fd, &ret, sizeof(ret))) == -1){
					log1(LWARN, "Read error: %s", 
						strerror(errno));
					ret = RET_ERR;
				}
			}
			break;
		}
	}
	if(i == _cmd_func_map_no){
		log1(LWARN, "No such command: %s", cmd);
		ret = RET_CMD_NOT_FOUND;
	}
	return ret;
}

static const char *get_cli_proto_error(struct cproto_if *self, int error_no)
{
	return get_proto_error(error_no);
}

static const char *get_proto_error(int error_no)
{
	if(error_no <= _return_mesg_no){
		return _return_mesg[error_no].mesg;
	}
	return NULL;
}


/* helper functions */

/* write with timeout */
static int write_to(int fd, struct timeval *timeout, 
		const void *buf, int size, size_t *ret_no)
{
	fd_set set;
	struct timeval dtimeout = {RW_TIMEOUT_SEC, 0};

	if(!timeout){
		timeout = &dtimeout;
	}	

	FD_ZERO(&set);
	FD_SET(fd, &set);

	select(fd+1, NULL, &set, NULL, timeout);
	if(FD_ISSET(fd, &set)){
		if((*ret_no=write(fd, buf, size)) == -1){
			log2(LWARN, "Write error to fd %d: %s", 
				fd, strerror(errno));
			return RET_WRITE_ERR;
		}else{
			return RET_OK;
		}
	}else{
		log1(LWARN, "Write timeout: %d", fd);
		return RET_TIMEOUT;
	}
}

/* read with timeout */
static int read_to(int fd, struct timeval *timeout, 
		void *buf, int size, size_t *ret_no)
{
	fd_set set;
	struct timeval dtimeout = {RW_TIMEOUT_SEC, 0};

	if(!timeout){
		timeout = &dtimeout;
	}	

	FD_ZERO(&set);
	FD_SET(fd, &set);

	select(fd+1, &set, NULL, NULL, timeout);
	if(FD_ISSET(fd, &set)){
		if((*ret_no=read(fd, buf, size)) == -1){
			log2(LWARN, "Read error from fd %d: %s", 
				fd, strerror(errno));
			return RET_READ_ERR;
		}else{
			return RET_OK;
		}
	}else{
		log1(LWARN, "Read timeout: %d", fd);
		return RET_TIMEOUT;
	}
}

