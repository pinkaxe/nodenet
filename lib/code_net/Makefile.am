
lib_LTLIBRARIES = libcode_net.la

libcode_net_la_SOURCES = \
			 code.c

libcode_net_la_LIBADD =

bin_PROGRAMS = code_test

code_test_LDADD = libcode_net.la ../../lib/wrap/libwrap.la ../../lib/util/libutil.la  -lpthread
code_test_SOURCES = code_test.c
