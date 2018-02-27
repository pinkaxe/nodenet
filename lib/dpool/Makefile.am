
lib_LTLIBRARIES = libdpool.la

libdpool_la_SOURCES = \
			 dpool.c \
			 dpool.h

libdpool_la_LIBADD =

bin_PROGRAMS = dpool_test

dpool_test_LDADD= ../util/libutil.la ../wrap/libwrap.la libdpool.la ../bitmap/libbitmap.la ../log/liblog.la -lpthread
dpool_test_SOURCES = dpool_test.c
