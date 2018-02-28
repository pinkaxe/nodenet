#ifndef __PROTOCOL_IF_H
#define __PROTOCOL_IF_H

struct sproto_if {
	int (*do_cmd)(struct sproto_if *self, int fd);
	const char *(*get_proto_error)(struct sproto_if *self, int error_no);
	void (*free)(struct sproto_if *self);
	/* notify protocol handlers if connection is succ/fail */
	void (*conn_res)(struct sproto_if *self, int fd, bool succ);
};

struct cproto_if {
	int (*do_cmd)(struct cproto_if *self, 
			int fd, const char *cmd, void *data);
	const char *(*get_proto_error)(struct cproto_if *self, int error_no);
	void (*free)(struct cproto_if *self);
};

#endif
