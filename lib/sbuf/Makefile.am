
lib_LTLIBRARIES = libsbuf.la

libsbuf_la_SOURCES = \
			 sbuf.c \
			 sbuf.h

libsbuf_la_LIBADD =

bin_PROGRAMS = sbuf_test

sbuf_test_LDADD= libsbuf.la ../util/libutil.la ../wrap/libwrap.la ../log/liblog.la  -lpthread
sbuf_test_SOURCES = sbuf_test.c
