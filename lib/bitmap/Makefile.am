
lib_LTLIBRARIES = libbitmap.la

libbitmap_la_SOURCES = \
			 bitmap.c \
			 bitmap.h

libbitmap_la_LIBADD =

bin_PROGRAMS = bitmap_test

bitmap_test_LDADD= ../util/libutil.la ../wrap/libwrap.la libbitmap.la libbitmap.la -lpthread
bitmap_test_SOURCES = bitmap_test.c
