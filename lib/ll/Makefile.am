
lib_LTLIBRARIES = libll.la

libll_la_SOURCES = \
			 ll.c \
			 ll.h

libll_la_LIBADD =

bin_PROGRAMS = ll_test

ll_test_LDADD= ../util/libutil.la ../wrap/libwrap.la libll.la -lpthread
ll_test_SOURCES = ll_test.c
