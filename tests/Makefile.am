
bin_PROGRAMS = sbuf_test task_pool_test sm_test ll_test bitmap_test dpool_test

#AM_LDFLAGS = -L../lib/arch -larch -L../lib/wrap -lwrap -L../lib/util -lutil -lpthread
AM_LDFLAGS = ../lib/arch/libarch.la ../lib/wrap/libwrap.la ../lib/util/libutil.la -lpthread
AM_DEPENDECIES = $(AM_LDFLAGS)
sbuf_test_SOURCES = sbuf_test.c
task_pool_test_SOURCES = task_pool_test.c
sm_test_SOURCES = sm_test.c
ll_test_SOURCES = ll_test.c
bitmap_test_SOURCES = bitmap_test.c
dpool_test_SOURCES = dpool_test.c
