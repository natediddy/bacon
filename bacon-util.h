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

#define BACON_SEC_MILLIS       1000
#define BACON_TOKEN_MAX        128
#define BACON_TOKEN_BUFFER_MAX 64

#define BACON_COLOR_BLACK   0
#define BACON_COLOR_RED     1
#define BACON_COLOR_GREEN   2
#define BACON_COLOR_YELLOW  3
#define BACON_COLOR_BLUE    4
#define BACON_COLOR_MAGENTA 5
#define BACON_COLOR_CYAN    6
#define BACON_COLOR_WHITE   7

#define bacon_new(type)        ((type *) bacon_malloc (sizeof (type)))
#define bacon_newa(type, size) ((type *) bacon_malloc (size))

#define bacon_free(p) \
  do {                \
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

typedef struct BaconQueryTokenList BaconQueryTokenList;

struct BaconQueryTokenList {
  char token[BACON_TOKEN_MAX];
  BaconQueryTokenList *next;
  BaconQueryTokenList *prev;
};

typedef unsigned long CmByte;

#ifdef BACON_DEBUG
void __bacon_debug (const char *file,
                    const char *func,
                    const int line,
                    const char *msg,
                    ...);
#endif
void bacon_error (const char *msg, ...);
void bacon_warn (const char *msg, ...);
void bacon_foutc (FILE *stream, const char c);
void bacon_outc (const char c);
void bacon_foutcco (FILE *stream, int color, bool bold, const char c);
void bacon_outcco (int color, bool bold, const char c);
void bacon_fout (FILE *stream, const char *msg, ...);
void bacon_foutln (FILE *stream, const char *msg, ...);
void bacon_foutco (FILE *stream, int color, bool bold, const char *msg, ...);
void bacon_foutlnco (FILE *stream,
                     int color,
                     bool bold,
                     const char *msg,
                     ...);
void bacon_out (const char *msg, ...);
void bacon_outln (const char *msg, ...);
void bacon_outco (int color, bool bold, const char *msg, ...);
void bacon_outlnco (int color, bool bold, const char *msg, ...);
void bacon_outi (const int level, const char *msg, ...);
void bacon_outlni (const int level, const char *msg, ...);
void bacon_msg (const char *msg, ...);
void bacon_color (int color, bool bold, FILE *stream);
void bacon_no_color (FILE *stream);
void *bacon_malloc (const size_t n);
void *bacon_realloc (void *ptr, const size_t n);
char *bacon_strdup (const char *str);
char *bacon_strndup (const char *str, const size_t n);
char *bacon_strf (const char *fmt, ...);
bool bacon_streq (const char *str1, const char *str2);
bool bacon_strstw (const char *str, const char *pre);
char bacon_tolower (char c);
bool bacon_isdigit (char c);
bool bacon_isspace (char c);
void bacon_strtolower (char *buf, size_t n, const char *str);
ssize_t bacon_strfposof (const char *str,
                         const char *query,
                         bool case_sensitive);
unsigned int bacon_stroccur (const char *str,
                             const char *query,
                             bool case_sensitive);
BaconQueryTokenList *bacon_query_token_list_new (const char *query);
void bacon_query_token_list_add (BaconQueryTokenList *query_tokens,
                                 const char *query);
void bacon_query_token_list_free (BaconQueryTokenList *query_tokens);
bool bacon_search (const char *content, BaconQueryTokenList *query_tokens);
void bacon_byte_str_format (char *buf, const size_t n, const CmByte bytes);
int bacon_int_from_str (const char *str);
bool bacon_nan_value (const double v);
void bacon_get_time_of_day (struct timeval *tv);
long bacon_get_millis (const struct timeval *start,
                       const struct timeval *end);

#endif /* BACON_UTIL_H */

