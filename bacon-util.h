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

#ifndef BACON_UTIL_H
#define BACON_UTIL_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "bacon.h"

#define BACON_BYTE_SYMBOL 'B'
#define BACON_KIBI_SYMBOL 'K'
#define BACON_MEBI_SYMBOL 'M'
#define BACON_GIBI_SYMBOL 'G'

#define BACON_SEC_MILLIS 1000

#define bacon_new(type)        ((type *) bacon_malloc (sizeof (type)))
#define bacon_newa(type, size) ((type *) bacon_malloc (size))

#define bacon_free(p) \
  do                  \
  {                   \
    if (!(p))         \
      break;          \
    free ((p));       \
    (p) = NULL;       \
  } while (false)

#define bacon_round(x) \
  (((x) >= 0) ? ((int) ((x) + 0.5)) : ((int) ((x) - 0.5)))

#ifdef BACON_DEBUG
# define bacon_debug(msg, ...) \
    __bacon_debug (__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#else
# define bacon_debug(msg, ...)
#endif

typedef unsigned long CmByte;

#ifdef BACON_DEBUG
void  __bacon_debug            (const char *file,
                                const char *func,
                                const int line,
                                const char *msg,
                                ...);
#endif
void  bacon_error              (const char *msg, ...);
void  bacon_warn               (const char *msg, ...);
void  bacon_fout               (FILE *stream, const char *msg, ...);
void  bacon_foutln             (FILE *stream, const char *msg, ...);
void  bacon_out                (const char *msg, ...);
void  bacon_outln              (const char *msg, ...);
void  bacon_outlni             (const int level, const char *msg, ...);
void  bacon_msg                (const char *msg, ...);
void *bacon_malloc             (const size_t n);
void *bacon_realloc            (void *ptr, const size_t n);
char *bacon_strdup             (const char *str);
char *bacon_strndup            (const char *str, const size_t n);
char *bacon_strf               (const char *fmt, ...);
bool  bacon_streq              (const char *str1, const char *str2);
bool  bacon_strstw             (const char *str, const char *pre);
void  bacon_byte_str_format    (char *buf,
                                const size_t n,
                                const CmByte bytes);
int   bacon_int_from_str       (const char *str);
bool  bacon_nan_value          (const double v);
void  bacon_get_time_of_day    (struct timeval *tv);
long  bacon_get_millis         (const struct timeval *start,
                                const struct timeval *end);

#endif /* BACON_UTIL_H */

