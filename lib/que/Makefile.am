
lib_LTLIBRARIES = libque.la

libque_la_SOURCES = \
			 que.c \
			 que.h

libque_la_LIBADD =


bin_PROGRAMS = que_test

que_test_LDADD = ../util/libutil.la ../wrap/libwrap.la libque.la ../log/liblog.la  -lpthread -lrt
que_test_SOURCES = que_test.c

