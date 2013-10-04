AC_INIT([bacon], [2.3], [me@nathanforbes.com],
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
    if test "x$CFLAGS" = "x" ; then
      CFLAGS="$LIBCURL_CFLAGS"
    else
      CFLAGS="$CFLAGS $LIBCURL_CFLAGS"
    fi
  fi
  if test "x$LIBCURL_LIBS" != "x" ; then
    if test "x$LIBS" = "x" ; then
      LIBS="$LIBCURL_LIBS"
    else
      LIBS="$LIBS $LIBCURL_LIBS"
    fi
  fi
  AC_MSG_NOTICE([found libcurl])
else
  AC_MSG_ERROR([libcurl not found!])
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
