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

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "bacon-util.h"

#define BACON_INDENT_SIZE 4

#define BACON_KIBIBYTE_VALUE ((CmByte) 1024LU)
#define BACON_MEBIBYTE_VALUE ((CmByte) 1048576LU)
#define BACON_GIBIBYTE_VALUE ((CmByte) 1073741824LU)

#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG  "DEBUG: "
#endif
#define BACON_ERROR_TAG  "error: "
#define BACON_WARN_TAG   "warning: "
#define BACON_NORMAL_TAG ""

extern const char *g_program_name;

#ifdef BACON_DEBUG
void
__bacon_debug (const char *file,
               const char *func,
               const int line,
               const char *msg,
               ...)
{
  va_list a;

  if (msg && *msg) {
    fprintf (stderr, "%s:", g_program_name);
    if (file && *file)
      fprintf (stderr, "%s:", file);
    if (func && *func)
      fprintf (stderr, "%s:", func);
    if (line >= 0)
      fprintf (stderr, "%i:", line);
    fputs (BACON_DEBUG_TAG, stderr);
    va_start (a, msg);
    vfprintf (stderr, msg, a);
    va_end (a);
  }
  fputc ('\n', stderr);
}
#endif

void
bacon_error (const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    fprintf (stderr, "%s: " BACON_ERROR_TAG, g_program_name);
    va_start (a, msg);
    vfprintf (stderr, msg, a);
    va_end (a);
  }
  fputc ('\n', stderr);
}

void
bacon_warn (const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    fprintf (stderr, "%s: " BACON_WARN_TAG, g_program_name);
    va_start (a, msg);
    vfprintf (stderr, msg, a);
    va_end (a);
  }
  fputc ('\n', stderr);
}

void
bacon_fout (FILE *stream, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stream, msg, a);
    va_end (a);
  }
}

void
bacon_foutln (FILE *stream, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stream, msg, a);
    va_end (a);
  }
  fputc ('\n', stream);
}

void
bacon_out (const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
}

void
bacon_outln (const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  fputc ('\n', stdout);
}

void
bacon_outlni (const int level, const char *msg, ...)
{
  int i;
  int j;
  va_list a;

  for (i = 0; i < level; ++i)
    for (j = 0; j < BACON_INDENT_SIZE; ++j)
      fputc (' ', stdout);

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  fputc ('\n', stdout);
}

void
bacon_msg (const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    fprintf (stdout, "%s: " BACON_NORMAL_TAG, g_program_name);
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  fputc ('\n', stdout);
}

void *
bacon_malloc (const size_t n)
{
  void *ptr;

  ptr = malloc (n);
  if (!ptr) {
    bacon_error (strerror (errno));
    exit (EXIT_FAILURE);
  }
  return ptr;
}

void *
bacon_realloc (void *ptr, const size_t n)
{
  void *pptr;

  pptr = realloc (ptr, n);
  if (!pptr) {
    bacon_error (strerror (errno));
    exit (EXIT_FAILURE);
  }
  return pptr;
}

char *
bacon_strdup (const char *str)
{
  size_t n;
  char *res;

  if (str) {
    n = strlen (str);
    res = bacon_newa (char, n + 1);
    memcpy (res, str, n);
    res[n] = '\0';
    return res;
  }
  return NULL;
}

char *
bacon_strndup (const char *str, const size_t n)
{
  char *res;

  if (str) {
    res = bacon_newa (char, n + 1);
    memcpy (res, str, n);
    res[n] = '\0';
    return res;
  }
  return NULL;
}

char *
bacon_strf (const char *fmt, ...)
{
  ssize_t n;
  size_t size;
  char *nstr;
  char *str;
  va_list a;

  size = strlen (fmt);
  if (!size)
    return NULL;

  str = bacon_newa (char, size);
  while (true) {
    va_start (a, fmt);
    n = vsnprintf (str, size, fmt, a);
    va_end (a);
    if (n > -1 && n < size)
      break;
    if (n > -1)
      size = n + 1;
    else
      size *= 2;
    nstr = (char *) bacon_realloc (str, size);
    str = nstr;
  }
  return str;
}

bool
bacon_streq (const char *str1, const char *str2)
{
  size_t n;

  n = strlen (str1);
  if ((n == strlen (str2)) && (memcmp (str1, str2, n) == 0))
    return true;
  return false;
}

bool
bacon_strstw (const char *str, const char *pre)
{
  size_t n;

  n = strlen (pre);
  if ((strlen (str) >= n) && (memcmp (str, pre, n) == 0))
    return true;
  return false;
}

bool
bacon_strhasc (const char *str, const char c)
{
  if (strchr (str, c))
    return true;
  return false;
}

void
bacon_byte_str_format (char *buf, const size_t n, const CmByte bytes)
{
  double x;
  char c;

  if (bytes < BACON_KIBIBYTE_VALUE)
    snprintf (buf, n, "%lu%c", bytes, BACON_BYTE_SYMBOL);
  else {
    if ((bytes / BACON_GIBIBYTE_VALUE) > 0) {
      x = ((double) bytes / (double) BACON_GIBIBYTE_VALUE);
      c = BACON_GIBI_SYMBOL;
    } else if ((bytes / BACON_MEBIBYTE_VALUE) > 0) {
      x = ((double) bytes / (double) BACON_MEBIBYTE_VALUE);
      c = BACON_MEBI_SYMBOL;
    } else {
      x = ((double) bytes / (double) BACON_KIBIBYTE_VALUE);
      c = BACON_KIBI_SYMBOL;
    }
    snprintf (buf, n, "%.1f%c", x, c);
  }
}

int
bacon_int_from_str (const char *str)
{
  int sign;
  long long n;
  long long g;

  n = 0LL;
  if (str && *str) {
    g = 0LL;
    sign = 1;
    while (isspace (*str))
      str++;
    if (*str == '-')
      sign = -1;
    while (*str) {
      if (isdigit (*str)) {
        g = g * 10LL + *str++ - '0';
        continue;
      }
      str++;
    }
    n = g * sign;
  }
  return ((int) n);
}

bool
bacon_nan_value (const double v)
{
  volatile double x;

  x = v;
  if (x != x)
    return true;
  return false;
}

void
bacon_get_time_of_day (struct timeval *tv)
{
  memset (tv, 0, sizeof (struct timeval));
  if (gettimeofday (tv, NULL) == -1) {
    bacon_error ("failed to get time of day (%s)", strerror (errno));
    exit (EXIT_FAILURE);
  }
}

long
bacon_get_millis (const struct timeval *start, const struct timeval *end)
{
  return (((end->tv_sec - start->tv_sec) * BACON_SEC_MILLIS) +
          ((end->tv_usec - start->tv_usec) / BACON_SEC_MILLIS));
}

