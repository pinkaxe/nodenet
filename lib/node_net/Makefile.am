
lib_LTLIBRARIES = libnode_net.la

libnode_net_la_SOURCES = \
			 types.h \
			 node.c \
			 node.h \
			 router.c \
			 router.h \
			 conn.c \
			 conn.h \
			 _conn.c \
			 _conn.h \
			 pkt.c \
			 pkt.h \
			 node_drivers/node_driver.c \
			 node_drivers/node_driver.h \
			 node_drivers/thread.c \
			 node_drivers/thread.h

#node_drivers/lproc.c /node_drivers/bin.c

libnode_net_la_LIBADD =

bin_PROGRAMS = nn_test
#
nn_test_LDADD = libnode_net.la ../../lib/wrap/libwrap.la ../../lib/util/libutil.la  -lpthread
nn_test_SOURCES = nn_test.c
