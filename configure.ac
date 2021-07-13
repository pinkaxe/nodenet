
AC_INIT([nodenet], [0.0.0], [])
#AC_PREREQ([2.61])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC
AC_PROG_LD
AM_PROG_AR
AC_PROG_LIBTOOL
#AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		lib/cragbag/Makefile
        lib/cragbag/log/Makefile
		lib/cragbag/sys/Makefile
		lib/cragbag/wrap/Makefile
		lib/cragbag/util/Makefile
		lib/cragbag/util/tests/Makefile
		lib/cragbag/db/Makefile
		lib/cragbag/file/Makefile
		lib/cragbag/que/Makefile
		lib/cragbag/ll/Makefile
        lib/cragbag/bitmap/Makefile
        lib/cragbag/dpool/Makefile
        lib/cragbag/link/Makefile
        lib/cragbag/sbuf/Makefile
		lib/cragbag/async_runner/Makefile

        lib/cragbag/sock_serv/Makefile

		lib/Makefile
		lib/nodenet/Makefile
		lib/nodenet/tests/Makefile
            ])

AC_ARG_ENABLE(debug, [  --enable-debug            enable debugging
        [[default=yes]]],, enable_debug=yes)
if test x"$enable_debug" = x"yes"; then
    CFLAGS="-O0 -g -pg -fno-inline -Wall "
else
    CFLAGS="-O2 -DNDEBUG "
fi
CFLAGS+=-I$PWD/lib

AC_OUTPUT
