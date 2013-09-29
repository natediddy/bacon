AC_INIT([bacon], [2.0], [me@nathanforbes.com],
        [bacon], [https://github.com/natediddy/bacon])
AC_CONFIG_SRCDIR([bacon.c])

AM_INIT_AUTOMAKE([-Wall no-define foreign])

AC_CONFIG_HEADERS([bacon-config.h])
AC_PROG_CC
AC_CONFIG_FILES([Makefile])

AC_CHECK_LIB(curl, curl_easy_init, [LIBS="$LIBS -lcurl"], [
    AC_ERROR([libcurl is required to build bacon, if you do not have it,
              you can get it from http://curl.haxx.se/download.html/ or
              from your system package manager (if you have one)])
])

AC_OUTPUT
