#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([bacon], [0.14.1], [me@nathanforbes.com])
AC_CONFIG_SRCDIR([bacon-env.h])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_easy_init])

# Checks for header files.
AC_CHECK_HEADERS([sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset mkdir strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
