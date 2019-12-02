
lib_LTLIBRARIES = libthread_pool.la

libthread_pool_la_SOURCES = \
			 thread_pool.c \
			 thread_pool.h

libthread_pool_la_LIBADD = -lpthread

bin_PROGRAMS = thread_pool_test

thread_pool_test_LDADD= libthread_pool.la  -lpthread ../util/libutil.la ../wrap/libwrap.la ../log/liblog.la
thread_pool_test_SOURCES = thread_pool_test.c
