
CRAGBAG_DIR = ../cragbag/

lib_LTLIBRARIES = libnodenet.la

libnodenet_la_SOURCES = \
			 types.h \
			 node.c \
			 node.h \
			 router.c \
			 router.h \
			 pkt.c \
			 pkt.h \
			 node_drivers/node_driver.c \
			 node_drivers/node_driver.h \
			 node_drivers/thread.c \
			 node_drivers/thread.h

#conn.c \ conn.h \ _conn.c \ _conn.h 

#node_drivers/lproc.c /node_drivers/bin.c

libnodenet_la_CPPFLAGS = -I$(CRAGBAG_DIR)
libnodenet_la_LIBADD = -lpthread

bin_PROGRAMS = nn_test2
#
#nn_test_LDADD = libnodenet.la $(CRAGBAG_DIR)/wrap/libwrap.la $(CRAGBAG_DIR)/que/libque.la $(CRAGBAG_DIR)/ll/libll.la $(CRAGBAG_DIR)/dpool/libdpool.la $(CRAGBAG_DIR)/bitmap/libbitmap.la $(CRAGBAG_DIR)/log/liblog.la -lrt -lpthread
#nn_test_SOURCES = nn_test.c
nn_test2_CPPFLAGS = -I$(CRAGBAG_DIR)
nn_test2_LDADD = libnodenet.la $(CRAGBAG_DIR)/wrap/libwrap.la $(CRAGBAG_DIR)/que/libque.la $(CRAGBAG_DIR)/ll/libll.la $(CRAGBAG_DIR)/dpool/libdpool.la $(CRAGBAG_DIR)/bitmap/libbitmap.la $(CRAGBAG_DIR)/log/liblog.la -lrt -lpthread
nn_test2_SOURCES = nn_test2.c
