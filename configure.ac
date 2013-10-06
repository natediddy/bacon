AC_INIT([bacon], [3.0], [me@nathanforbes.com],
        [bacon], [https://github.com/natediddy/bacon])
AC_CONFIG_SRCDIR([bacon.c])

AM_INIT_AUTOMAKE([-Wall no-define foreign])

AC_CONFIG_HEADERS([bacon-config.h])
AC_PROG_CC
AC_CONFIG_FILES([Makefile])

AC_CONFIG_MACRO_DIR([m4])

PKG_PROG_PKG_CONFIG

CURL_MINIMUM=7.15.4
AC_SUBST(CURL_MINIMUM)

has_libcurl="no"
PKG_CHECK_MODULES(LIBCURL,
                  [libcurl >= $CURL_MINIMUM],
                  [has_libcurl="yes"],
                  [has_libcurl="no"])

if test "x$has_libcurl" != "xno" ; then
  if test "x$LIBCURL_CFLAGS" != "x" ; then
    CFLAGS="$CFLAGS $LIBCURL_CFLAGS"
  fi
  if test "x$LIBCURL_LIBS" != "x" ; then
    LIBS="$LIBS $LIBCURL_LIBS"
  fi
else
  AC_MSG_ERROR([Could not find libcurl libraries!])
fi

GTK_MINIMUM=3.2.0
AC_SUBST(GTK_MINIMUM)
GLIB_MINIMUM=2.28.0
AC_SUBST(GLIB_MINIMUM)
GIO_MINIMUM=2.26.0
AC_SUBST(GIO_MINIMUM)

AC_ARG_WITH([gtk],
            AC_HELP_STRING([--with-gtk], [Compile with the GTK+ UI]),
            [with_gtk="yes"],
            [with_gtk="no"])

if test "x$with_gtk" = "xyes" ; then
  PKG_CHECK_MODULES([GTK],
                    [gtk+-3.0 >= $GTK_MINIMUM
                     glib-2.0 >= $GLIB_MINIMUM
                     gio-2.0 >= $GIO_MINIMUM],
                    [has_gtk="yes"],
                    [has_gtk="no"])
  if test "x$has_gtk" = "xyes" ; then
    if test "x$GTK_CFLAGS" != "x" ; then
      CFLAGS="$CFLAGS $GTK_CFLAGS -DBACON_USING_GTK"
    fi
    if test "x$GTK_LIBS" != "x" ; then
      LIBS="$LIBS $GTK_LIBS"
    fi
  else
    AC_MSG_ERROR([Could not find requested GTK3 libraries!])
  fi
else
  AC_MSG_NOTICE([Not using GTK3])
fi

enable_debugging="no"
AC_ARG_ENABLE([debug],
              AC_HELP_STRING([--enable-debug], [Enable debugging]),
              [enable_debugging="yes"],
              [])
if test "x$enable_debugging" != "xno" ; then
  CFLAGS="$CFLAGS -DBACON_DEBUG"
fi


os_string=""
AC_CANONICAL_HOST
BACON_OS_STRING="$host_os"
AC_SUBST(BACON_OS_STRING)
CFLAGS="$CFLAGS -DBACON_OS_STRING=\"$BACON_OS_STRING\""

AC_OUTPUT

echo "

Configuration:

  Source location:     ${srcdir}

  Compiler:            ${CC}

  CFLAGS:              ${CFLAGS}

  LIBS:                ${LIBS}

  Operating System:    ${BACON_OS_STRING}

"
