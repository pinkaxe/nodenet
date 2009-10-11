
lib_LTLIBRARIES = libcode_net.la

libcode_net_la_SOURCES = \
			 elem.c \
			 elem_io.c \
			 grp.c \
			 cmd.c \
			 cn.c \
			 router.c \
			 router_io.c \
			 elem_types/elem_type.c \
			 elem_types/thread.c \
			 elem_types/lproc.c \
			 elem_types/bin.c

libcode_net_la_LIBADD =

bin_PROGRAMS = cn_test
#
cn_test_LDADD = libcode_net.la ../../lib/wrap/libwrap.la ../../lib/util/libutil.la  -lpthread
cn_test_SOURCES = cn_test.c
