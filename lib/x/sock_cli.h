#ifndef __SOCK_CLI_H
#define __SOCK_CLI_H

struct client_handle;

struct client_handle *client_create(struct cproto_if *ph, char *host, int port);
int client_do(struct client_handle *ch, const char *cmd, void *data);
int client_stop(struct client_handle *ch);

#endif
