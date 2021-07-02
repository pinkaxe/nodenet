
AC_INIT([node_net], [0.0.0], [])
#AC_PREREQ([2.61])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC
AC_PROG_LD
AM_PROG_AR
AC_PROG_LIBTOOL
#AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		external/cragbag/lib/Makefile
        external/cragbag/lib/log/Makefile
		external/cragbag/lib/sys/Makefile
		external/cragbag/lib/wrap/Makefile
		external/cragbag/lib/util/Makefile
		external/cragbag/lib/util/tests/Makefile
		external/cragbag/lib/db/Makefile
		external/cragbag/lib/file/Makefile
		external/cragbag/lib/que/Makefile
		external/cragbag/lib/ll/Makefile
        external/cragbag/lib/bitmap/Makefile
        external/cragbag/lib/dpool/Makefile
        external/cragbag/lib/link/Makefile
        external/cragbag/lib/sbuf/Makefile

        external/cragbag/lib/sock_serv/Makefile

		lib/Makefile
		lib/node_net/Makefile
		lib/node_net/tests/Makefile
		external/cragbag/lib/async_runner/Makefile
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
