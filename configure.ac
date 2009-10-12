
AC_INIT([myproj], [0.0.0], [])
#AC_PREREQ([2.61])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_LD
AC_PROG_LIBTOOL
#AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		lib/Makefile
		lib/sys/Makefile
		lib/wrap/Makefile
		lib/util/Makefile
		lib/util/tests/Makefile
		lib/db/Makefile
		lib/node_net/Makefile
            ])

AC_ARG_ENABLE(debug, [  --enable-debug            enable debugging
        [[default=yes]]],, enable_debug=yes)
if test x"$enable_debug" = x"yes"; then
    CFLAGS="-O0 -g -fno-inline "
else
    CFLAGS="-O3 -DNDEBUG "
fi
CFLAGS+=-I$PWD/lib

#AC_CONFIG_SUBDIRS([lib/util])


# conditionals 

#AM_CONDITIONAL([PLATFORM_SIGMA8634], [test "$checkfor_sigma8634" = "yes"])
#AM_CONDITIONAL([PLATFORM_TI6467], [test "$checkfor_ti6467" = "yes"])
#AM_CONDITIONAL([PLATFORM_DESKTOP], [test "$checkfor_desktop" = "yes"])
#
#AM_CONDITIONAL([DIRECTFB_VERS], [test "$DIRECTFB_VERS" != ""])
#AM_CONDITIONAL([V4L2_VERS], [test "$V4L2_VERS" != ""])
#AM_CONDITIONAL([OPERA_VERS], [test "$OPERA_VERS" != ""])
#
#
## store enviroment variables
#
#AC_SUBST([ARCH], [$ARCH])
#AC_SUBST([PLATFORM], [$PLATFORM])
#AC_SUBST([CC_VERS], [$CC_VERS])
#AC_SUBST([CUST_DIR], [$CUST_DIR])
#AC_SUBST([PLAYER_DIR], [$PLAYER_DIR])
#AC_SUBST([PLAYER_CUSTOMER_DIR], [$PLAYER_CUSTOMER_DIR])
#PLAYER_CUSTOMER_DIR_STRIP=$(echo $PLAYER_CUSTOMER_DIR | sed 's/^ *\/*Customer//')
#AC_SUBST([PLAYER_CUSTOMER_DIR_STRIP], [$PLAYER_CUSTOMER_DIR_STRIP])
#
#AC_SUBST([KERNEL_SRC], [$KERNEL_SRC])
#AC_SUBST([OPERA_VERS], [$OPERA_VERS])
#AC_SUBST([MRUA_DIR], [$MRUA_DIR])
#AC_SUBST([MRUA_VERS], [$MRUA_VERS])
#
#AC_SUBST([DIRECTFB_VERS], [$DIRECTFB_VERS])
#
#AC_SUBST([BUILD_APPS], [$BUILD_APPS])
#
AC_OUTPUT
#
#AC_MSG_RESULT([
#Configured for:
#    ARCH.................$ARCH
#    PLATFORM.............$PLATFORM
#    CC_VERS..............$CC_VERS
#    CUST_DIR.............$CUST_DIR
#    KERNEL_SRC...........$KERNEL_SRC
#
#    PLAYER_DIR...........$PLAYER_DIR
#    PLAYER_CUSTOMER_DIR..$PLAYER_CUSTOMER_DIR
#    MRUA_DIR.............$MRUA_DIR
#    MRUA_VERS............$MRUA_VERS
#
#    OPERA_VERS...........$OPERA_VERS
#    DIRECTFB_VERS........$DIRECTFB_VERS
#    V4L2_VERS............$V4L2_VERS
#
#    BUILD_APPS...........$BUILD_APPS
#    ]);
#
#
#
#
#
#
