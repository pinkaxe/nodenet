
lib_LTLIBRARIES = libasync_runner.la

libasync_runner_la_SOURCES = \
			 async_runner.c \
			 async_runner.h

libasync_runner_la_LIBADD = -lpthread

bin_PROGRAMS = async_runner_test

async_runner_test_LDADD= libasync_runner.la  -lpthread ../util/libutil.la ../wrap/libwrap.la ../log/liblog.la
async_runner_test_SOURCES = async_runner_test.c
