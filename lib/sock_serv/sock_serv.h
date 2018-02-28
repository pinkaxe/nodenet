#ifndef __SOCK_SERV_H
#define __SOCK_SERV_H

struct server_handle;

struct server_handle *server_create(struct sproto_if *ph, int port, int max);
int server_stop(struct server_handle *sh);


#endif
