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

#ifndef BACON_SYS_H
#define BACON_SYS_H

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#undef __BACON_OS_DEFINED__

#if defined (_AIX)
# define BACON_OS_AIX
# define BACON_OS_STRING "AIX"
# define __BACON_OS_DEFINED__
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__APPLE__)
#  include <TargetConditionals.h>
#  if defined (TARGET_OS_MAC) && TARGET_OS_MAC
#   define BACON_OS_OSX
#   define BACON_OS_STRING "Mac OSX"
#   define __BACON_OS_DEFINED__
#  endif
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (WIN64) || defined (_WIN64) || defined (__WIN64__)
#  define BACON_OS_WIN64
#  define BACON_OS_STRING "Windows (64-bit)"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (WIN32) || defined (_WIN32) || \
     defined (__WIN32__) || defined (__NT__)
#  if defined (WINCE) || defined (_WIN32_WCE)
#   define BACON_OS_WINCE
#   define BACON_OS_STRING "Windows CE"
#  else
#   define BACON_OS_WIN32
#   define BACON_OS_STRING "Windows (32-bit)"
#  endif
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (MSDOS)
#  define BACON_OS_MSDOS
#  define BACON_OS_STRING "MS-DOS"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__EMX__)
#  define BACON_OS_OS2
#  define BACON_OS_STRING "OS/2"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__sun) && defined (__SVR4)
#  define BACON_OS_SOLARIS
#  define BACON_OS_STRING "Solaris"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (hpux) || defined (__hpux)
#  define BACON_OS_HPUX
#  define BACON_OS_STRING "HP/UX"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__linux__) || defined (__linux)
#  define BACON_OS_LINUX
#  define BACON_OS_STRING "Linux"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__FreeBSD__)
#  define BACON_OS_FREEBSD
#  define BACON_OS_STRING "FreeBSD"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__DragonFly__)
#  define BACON_OS_DRAGONFLY
#  define BACON_OS_STRING "DragonFlyBSD"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__NetBSD__)
#  define BACON_OS_NETBSD
#  define BACON_OS_STRING "NetBSD"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__OpenBSD__)
#  define BACON_OS_OPENBSD
#  define BACON_OS_STRING "OpenBSD"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# if defined (__bsdi__)
#  define BACON_OS_BSDI
#  define BACON_OS_STRING "BSD"
#  define __BACON_OS_DEFINED__
# endif
#endif

#ifndef __BACON_OS_DEFINED__
# error "Bacon is not supported on this OS!"
#endif

#if defined (BACON_OS_WIN64) || \
    defined (BACON_OS_WIN32) || \
    defined (BACON_OS_WINCE)
# define BACON_OS_WINDOWS
#endif

#if defined (BACON_OS_DRAGONFLY) || \
    defined (BACON_OS_FREEBSD) || \
    defined (BACON_OS_SOLARIS) || \
    defined (BACON_OS_OSX) || \
    defined (BACON_OS_LINUX) || \
    defined (BACON_OS_HPUX) || \
    defined (BACON_OS_NETBSD) || \
    defined (BACON_OS_OPENBSD) || \
    defined (BACON_OS_BSDI) || \
    defined (BACON_OS_AIX)
# define BACON_OS_UNIX
#endif

#undef NAMESPACE_TWEAKS
#if defined (BACON_OS_SOLARIS) || \
    defined (BACON_OS_LINUX) || \
    defined (__GLIBC__)
# define NAMESPACE_TWEAKS
#endif

#ifdef NAMESPACE_TWEAKS
# define _XOPEN_SOURCE 500
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined (alloca)
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined (BACON_OS_AIX)
#  define alloca __alloca
# elif defined (BACON_OS_WINDOWS)
#  include <malloc.h>
#  ifndef alloca
#   define alloca _alloca
#  endif
# elif !defined (HAVE_ALLOCA)
#  ifdef __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

#undef __BaconBooleanType__

#ifndef __cplusplus
# if defined (HAVE_STDBOOL_H) && defined (HAVE__BOOL)
#  include <stdbool.h>
#  define __BaconBooleanType__ _Bool
# else
#  define __BaconBooleanType__ char
# endif
#else
# define __BaconBooleanType__ bool
#endif

typedef __BaconBooleanType__ BaconBoolean;

#ifdef false
# define BACON_FALSE false
#else
# define BACON_FALSE 0
#endif

#ifdef true
# define BACON_TRUE true
#else
# define BACON_TRUE (!BACON_FALSE)
#endif

#ifndef S_ISDIR
# define S_ISDIR(m) (((m) & (_S_IFMT)) == (_S_IFDIR))
#endif
#ifndef S_ISREG
# define S_ISREG(m) (((m) & (_S_IFMT)) == (_S_IFREG))
#endif

#ifdef HAVE_DIRECT_H
# include <direct.h>
# define bacon_mkdir(path) _mkdir (path)
#else
# define bacon_mkdir(path) mkdir (path, S_IRWXU)
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
# define bacon_delete_file(path) (unlink (path) == 0)
# define bacon_getcwd(buffer, n) (getcwd (buffer, n) != NULL)
#else
# ifdef HAVE_WINDOWS_H
#  include <windows.h>
#  define bacon_delete_file(path) (DeleteFile (path))
#  define bacon_getcwd(buffer, n) (GetCurrentDirectory (n, buffer) != 0)
# else
#  error "Failed to find suitable ways of deleting files " \
         "and getting the current working directory"
# endif
#endif

#endif /* BACON_SYS_H */

