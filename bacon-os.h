/*
 * Copyright (C) 2013 Nathan Forbes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

