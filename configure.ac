AC_INIT(
  [bacon],
  [3.0],
  [me@nathanforbes.com],
  [bacon],
  [https://github.com/natediddy/bacon],
)

AC_MSG_NOTICE([Configuring for $PACKAGE_NAME $PACKAGE_VERSION])

AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([bacon.c])
AM_INIT_AUTOMAKE([-Wall no-define foreign])
AC_CONFIG_HEADERS([bacon-config.h])
AC_CONFIG_FILES([Makefile])

AC_PROG_CC
AM_PROG_CC_C_O

AS_IF(
  [test x$GCC != x],
  [CFLAGS="$CFLAGS -Wall"],
  []
)

AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE

AC_HEADER_STDBOOL
AC_CHECK_HEADERS([direct.h unistd.h sys/time.h sys/ioctl.h windows.h])

AC_TYPE_LONG_LONG_INT
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_ARG_ENABLE(
  [debug],
  [AS_HELP_STRING([--enable-debug], [Enable debugging])
AS_HELP_STRING([--disable-debug], [Disable debugging (DEFAULT)])]
)

AS_IF(
  [test x$enable_debug = xyes],
  [AC_DEFINE([BACON_DEBUG], [1], [Define if debugging is enabled])],
  []
)

m4_include([pkg.m4])

PKG_PROG_PKG_CONFIG

libcurl_minimum=7.15.4

PKG_CHECK_MODULES(
  [libcurl],
  [libcurl >= $libcurl_minimum],
  [CFLAGS="$CFLAGS $libcurl_CFLAGS"
   LIBS="$LIBS $libcurl_LIBS"],
  [AC_MSG_ERROR([failed to find required libcurl libraries!])]
)

AC_ARG_WITH(
  [gtk],
  [AS_HELP_STRING([--with-gtk], [Compile with GTK+ interface])
AS_HELP_STRING([--without-gtk], [Compile without GTK+ interface [DEFAULT]])]
)

gtk_minimum=3.2.0
glib_minimum=2.28.0
gio_minimum=2.26.0

AS_IF(
  [test x$with_gtk = xyes],
  [PKG_CHECK_MODULES([gtk],
    [gtk+-3.0 >= $gtk_minimum
     glib-2.0 >= $glib_minimum
     gio-2.0 >= $gio_minimum],
    [CFLAGS="$CFLAGS $gtk_CFLAGS"
     LIBS="$LIBS $gtk_LIBS"
     AC_DEFINE([BACON_GTK], [1], [Define if compiling with GTK+ interface])
     AC_MSG_NOTICE([compiling with GTK+ interface])],
    [AC_MSG_ERROR([failed to find GTK+ libraries!])],
  )],
  []
)

# squeeze function from curl project
AC_DEFUN(
  [BACON_SQUEEZE],
  [_sqz_result=""
   eval _sqz_input=\$$1
   for _sqz_token in $_sqz_input; do
     if test -z "$_sqz_result"; then
       _sqz_result="$_sqz_token"
     else
       _sqz_result="$_sqz_result $_sqz_token"
     fi
   done
   eval $1=\$_sqz_result]
)

AS_IF(
  [test x$target != x],
  [AC_DEFINE_UNQUOTED(
    [BACON_TARGET],
    ["$target"],
    [Define to the target system]
  )],
  []
)

BACON_SQUEEZE(CFLAGS)
BACON_SQUEEZE(DEFS)
BACON_SQUEEZE(LIBS)

AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([Configuration for $PACKAGE_NAME $PACKAGE_VERSION:])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Source location: $srcdir])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Compiler:        $CC])
AC_MSG_RESULT([])
AC_MSG_RESULT([  CFLAGS:          $CFLAGS])
AC_MSG_RESULT([])
AC_MSG_RESULT([  DEFS:            $DEFS])
AC_MSG_RESULT([])
AC_MSG_RESULT([  LIBS:            $LIBS])
AC_MSG_RESULT([])
