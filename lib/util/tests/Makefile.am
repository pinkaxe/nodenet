
bin_PROGRAMS = sbuf_test bitmap_test dpool_test ll_test que_test

#AM_LDFLAGS = -L../lib/sys -lsys -L../lib/wrap -lwrap -L../lib/util -lutil -lpthread
AM_LDFLAGS = ../../wrap/libwrap.la ../../util/libutil.la -lpthread
AM_DEPENDECIES = $(AM_LDFLAGS)
sbuf_test_SOURCES = sbuf_test.c
ll_test_SOURCES = ll_test.c
bitmap_test_SOURCES = bitmap_test.c
dpool_test_SOURCES = dpool_test.c
que_test_SOURCES = que_test.c
