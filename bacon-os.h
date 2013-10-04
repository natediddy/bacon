/*
 * bacon - A command line tool for viewing/downloading CyanogenMod ROMs
 *         for Android devices.
 *
 * Copyright (C) 2013  Nathan Forbes
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACON_OS_H
#define BACON_OS_H

#undef __BACON_OS_DEFINED__

#if defined (__APPLE__)
# include <TargetConditionals.h>
# if defined (TARGET_OS_MAC) && TARGET_OS_MAC
#  define BACON_OS_OSX
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (WIN64) || defined (_WIN64) || defined (__WIN64__)
#  define BACON_OS_WIN64
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (WIN32) || defined (_WIN32) || \
     defined (__WIN32__) || defined (__NT__)
#  if defined (WINCE) || defined (_WIN32_WCE)
#   define BACON_OS_WINCE
#  else
#   define BACON_OS_WIN32
#  endif
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__sun) || defined (sun)
#  define BACON_OS_SOLARIS
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (hpux) || defined (__hpux)
#  define BACON_OS_HPUX
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__linux__) || defined (__linux)
#  define BACON_OS_LINUX
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__FreeBSD__)
#  define BACON_OS_FREEBSD
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__DragonFly__)
#  define BACON_OS_DRAGONFLY
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__NetBSD__)
#  define BACON_OS_NETBSD
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__OpenBSD__)
#  define BACON_OS_OPENBSD
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__bsdi__)
#  define BACON_OS_BSDI
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# error This program is not supported on this OS
#endif

#if defined (BACON_OS_WIN64) || \
    defined (BACON_OS_WIN32) || \
    defined (BACON_OS_WINCE)
# define BACON_OS_WINDOWS
#endif

#ifndef BACON_OS_WINDOWS
# define BACON_OS_UNIX
#endif

#if defined (BACON_OS_UNIX) && !defined (BACON_OS_LINUX)
# define BACON_OS_BSD
#endif

#endif /* BACON_OS_H */

