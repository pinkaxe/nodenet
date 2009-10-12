
lib_LTLIBRARIES = libnode_net.la

libnode_net_la_SOURCES = \
			 node.c \
			 node_io.c \
			 router.c \
			 router_io.c \
			 grp.c \
			 cmd.c \
			 nn.c \
			 node_types/node_type.c \
			 node_types/thread.c \
			 node_types/lproc.c \
			 node_types/bin.c

libnode_net_la_LIBADD =

bin_PROGRAMS = nn_test
#
nn_test_LDADD = libnode_net.la ../../lib/wrap/libwrap.la ../../lib/util/libutil.la  -lpthread
nn_test_SOURCES = nn_test.c
