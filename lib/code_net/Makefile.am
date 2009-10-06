
lib_LTLIBRARIES = libcode_net.la

libcode_net_la_SOURCES = \
			 net.c \
			 elem.c \
			 grp.c \
			 cn.c \
			 run.c \
			 dispatchers/thread.c \
			 dispatchers/lproc.c \
			 dispatchers/bin.c

libcode_net_la_LIBADD =

bin_PROGRAMS = cn_test
#
cn_test_LDADD = libcode_net.la ../../lib/wrap/libwrap.la ../../lib/util/libutil.la  -lpthread
cn_test_SOURCES = cn_test.c
