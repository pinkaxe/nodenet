
AC_INIT([node_net], [0.0.0], [])
#AC_PREREQ([2.61])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_LD
AM_PROG_AR
AC_PROG_LIBTOOL
#AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		lib/Makefile
        lib/log/Makefile
		lib/sys/Makefile
		lib/wrap/Makefile
		lib/util/Makefile
		lib/util/tests/Makefile
		lib/db/Makefile
		lib/file/Makefile
		lib/thread_pool/Makefile
		lib/que/Makefile
		lib/ll/Makefile
        lib/bitmap/Makefile
        lib/dpool/Makefile
        lib/link/Makefile
        lib/sbuf/Makefile

        lib/sock_serv/Makefile

		lib/node_net/Makefile
		lib/node_net/tests/Makefile
		lib/async_runner/Makefile
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
