AC_PREREQ([2.50])
AC_INIT(bacon, 0.20.5)
AC_CONFIG_SRCDIR([src])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

AM_MAINTAINER_MODE

AC_PROG_CXX
AC_PROG_CC

AC_SYS_LARGEFILE

AC_CHECK_LIB(curl, curl_easy_init, [LIBS="$LIBS -lcurl"], [
  AC_ERROR(libcurl is required to build bacon, if you do not have it,
           you can get it from http://curl.haxx.se/download.html/ or
           from your system package manager (if you have one))
])

AC_ARG_VAR(BACON_ROM_HISTORY_DEFAULT,
           declares the default amount of ROMs to search for)
AC_ARG_VAR(BACON_ROM_HISTORY_MAX,
           declares the maximum number of ROMs to search for)

if test x"$BACON_ROM_HISTORY_DEFAULT" = x; then
  BACON_ROM_HISTORY_DEFAULT=3
fi

if test x"$BACON_ROM_HISTORY_MAX" = x; then
  BACON_ROM_HISTORY_MAX=10
fi

if test x"$MACHTYPE" != x; then
  CXXFLAGS="$CXXFLAGS -DBACON_MACHTYPE=\\\"$MACHTYPE\\\""
fi

AC_CHECK_HEADERS([sys/ioctl.h unistd.h])

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset mkdir strrchr])

AC_CONFIG_FILES([
Makefile
src/Makefile
docs/Makefile
])

AC_OUTPUT
