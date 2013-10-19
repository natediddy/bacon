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

#include "bacon.h"

#include <string.h>

#include "bacon-ctype.h"
#include "bacon-out.h"
#include "bacon-str.h"
#include "bacon-util.h"

#define BACON_BYTE_SYMBOL 'B'
#define BACON_KIBI_SYMBOL 'K'
#define BACON_MEBI_SYMBOL 'M'
#define BACON_GIBI_SYMBOL 'G'

#define BACON_KIBI_VALUE 1024LU
#define BACON_MEBI_VALUE 1048576LU
#define BACON_GIBI_VALUE 1073741824LU

static BaconBoolean
bacon_ceq (char c1, char c2, BaconBoolean case_sensitive)
{
  char cc1;
  char cc2;

  if (!case_sensitive) {
    cc1 = bacon_tolower (c1);
    cc2 = bacon_tolower (c2);
  } else {
    cc1 = c1;
    cc2 = c2;
  }

  if (cc1 == cc2)
    return BACON_TRUE;
  return BACON_FALSE;
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
bacon_strndup (const char *str, size_t n)
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
  while (BACON_TRUE) {
    va_start (a, fmt);
    n = vsnprintf (str, size, fmt, a);
    va_end (a);
    if ((n > -1) && (n < size))
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

BaconBoolean
bacon_streq (const char *str1, const char *str2)
{
  size_t n;

  n = strlen (str1);
  if ((n == strlen (str2)) && (memcmp (str1, str2, n) == 0))
    return BACON_TRUE;
  return BACON_FALSE;
}

BaconBoolean
bacon_streqci (const char *str1, const char *str2)
{
  size_t n;

  n = strlen (str1);
  if (n == strlen (str2)) {
    char buf1[n + 1];
    char buf2[n + 1];
    bacon_strtolower (buf1, n, str1);
    buf1[n] = '\0';
    bacon_strtolower (buf2, n, str2);
    buf2[n] = '\0';
    if (memcmp (buf1, buf2, n) == 0)
      return BACON_TRUE;
  }
  return BACON_FALSE;
}

BaconBoolean
bacon_strstw (const char *str, const char *pre)
{
  size_t n;

  n = strlen (pre);
  if ((strlen (str) >= n) && (memcmp (str, pre, n) == 0))
    return BACON_TRUE;
  return BACON_FALSE;
}

BaconBoolean
bacon_strew (const char *str, const char *suf, BaconBoolean case_sensitive)
{
  size_t n_str;
  size_t n_suf;
  ssize_t i;
  ssize_t j;

  n_str = strlen (str);
  n_suf = strlen (suf);

  if (n_str >= n_suf) {
    for (i = n_str - 1, j = n_suf - 1; ((i >= 0) && (j >= 0)); --i, --j) {
      if (!bacon_ceq (str[i], suf[j], case_sensitive))
        break;
      if ((j == 0) && (i == (n_str - n_suf)))
        return BACON_TRUE;
    }
  }
  return BACON_FALSE;
}

void
bacon_strtolower (char *buf, size_t n, const char *str)
{
  size_t x;

  for (x = 0; x < n; ++x)
    buf[x] = bacon_tolower (str[x]);
  buf[n] = '\0';
}

ssize_t
bacon_strfposof (const char *str,
                 const char *query,
                 BaconBoolean case_sensitive)
{
  ssize_t i;
  ssize_t j;
  ssize_t s;

  for (i = 0; str[i]; ++i) {
    if (bacon_ceq (str[i], query[0], case_sensitive)) {
      s = i;
      for (j = 1, i = i + 1; query[j] && str[i]; ++j, ++i)
        if (!bacon_ceq (str[i], query[j], case_sensitive))
          break;
      if (!query[j])
        return s;
    }
  }
  return -1;
}

unsigned int
bacon_stroccurs (const char *str,
                 const char *query,
                 BaconBoolean case_sensitive)
{
  unsigned int c;
  size_t n_str;
  size_t n_query;
  char *p;
  char *s;

  if (!str || !*str || !query || !*query)
    return 0;

  n_str = strlen (str);
  char strbuf[n_str + 1];

  if (!case_sensitive)
    bacon_strtolower (strbuf, n_str, str);
  else {
    strncpy (strbuf, str, n_str);
    strbuf[n_str] = '\0';
  }

  n_query = strlen (query);
  char querybuf[n_query + 1];

  if (!case_sensitive)
    bacon_strtolower (querybuf, n_query, query);
  else {
    strncpy (querybuf, query, n_query);
    querybuf[n_query] = '\0';
  }

  c = 0;
  p = strstr (strbuf, querybuf);
  while (p) {
    c++;
    s = p + n_query;
    p = strstr (s, querybuf);
  }
  return c;
}

void
bacon_strbytes (char *buf, size_t n, unsigned long bytes)
{
  double x;
  char c;

  if (bytes < BACON_KIBI_VALUE)
    snprintf (buf, n, "%lu%c", bytes, BACON_BYTE_SYMBOL);
  else {
    if ((bytes / BACON_GIBI_VALUE) > 0) {
      x = ((double) bytes / (double) BACON_GIBI_VALUE);
      c = BACON_GIBI_SYMBOL;
    } else if ((bytes / BACON_MEBI_VALUE) > 0) {
      x = ((double) bytes / (double) BACON_MEBI_VALUE);
      c = BACON_MEBI_SYMBOL;
    } else {
      x = ((double) bytes / (double) BACON_KIBI_VALUE);
      c = BACON_KIBI_SYMBOL;
    }
    snprintf (buf, n, "%.1f%c", x, c);
  }
}

int
bacon_strtoint (const char *str)
{
  int sign;
  long long n;
  long long g;

  n = 0LL;
  if (str && *str) {
    g = 0LL;
    sign = 1;
    while (bacon_isspace (*str))
      str++;
    if (*str == '-')
      sign = -1;
    while (*str) {
      if (bacon_isdigit (*str)) {
        g = g * 10LL + *str++ - '0';
        continue;
      }
      str++;
    }
    n = g * sign;
  }
  return ((int) n);
}

