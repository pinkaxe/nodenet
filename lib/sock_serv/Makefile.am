
lib_LTLIBRARIES = libsock_serv.la

libsock_serv_la_SOURCES = \
						  protocols/protocol_if.h \
						  protocols/filetrans.h \
						  protocols/filetrans.c \
						  sock_serv.c \
						  sock_serv.h

libsock_serv_la_LIBADD =

bin_PROGRAMS = sock_serv_test

sock_serv_test_LDADD= libsock_serv.la ../file/libfile.la ../async_runner/libasync_runner.la ../log/liblog.la -lpthread
sock_serv_test_SOURCES = sock_serv_test.c
