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

#include <errno.h>
#include <string.h>

#include "bacon-os.h"
#include "bacon-util.h"

#define BACON_INDENT_SIZE      4
#define BACON_COLOR_STRING_MAX 128
#define BACON_KIBIBYTE_VALUE   ((CmByte) 1024LU)
#define BACON_MEBIBYTE_VALUE   ((CmByte) 1048576LU)
#define BACON_GIBIBYTE_VALUE   ((CmByte) 1073741824LU)

#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG "DEBUG: "
#endif
#define BACON_ERROR_TAG  "error: "
#define BACON_WARN_TAG   "warning: "
#define BACON_NORMAL_TAG ""

#ifdef BACON_OS_UNIX
# define BACON_COLOR_CODE_BOLD    "\x1B[1m"
# define BACON_COLOR_CODE_NONE    "\x1B[0m"
# define BACON_COLOR_CODE_BLACK   "\x1B[30m"
# define BACON_COLOR_CODE_RED     "\x1B[31m"
# define BACON_COLOR_CODE_GREEN   "\x1B[32m"
# define BACON_COLOR_CODE_YELLOW  "\x1B[33m"
# define BACON_COLOR_CODE_BLUE    "\x1B[34m"
# define BACON_COLOR_CODE_MAGENTA "\x1B[35m"
# define BACON_COLOR_CODE_CYAN    "\x1B[36m"
# define BACON_COLOR_CODE_WHITE   "\x1B[37m"
#else
# define BACON_COLOR_CODE_BOLD
# define BACON_COLOR_CODE_NORMAL
# define BACON_COLOR_CODE_BLACK
# define BACON_COLOR_CODE_RED
# define BACON_COLOR_CODE_GREEN
# define BACON_COLOR_CODE_YELLOW
# define BACON_COLOR_CODE_BLUE
# define BACON_COLOR_CODE_MAGENTA
# define BACON_COLOR_CODE_CYAN
# define BACON_COLOR_CODE_WHITE
#endif

extern const char *g_program_name;
extern bool        g_use_color;

static bool
bacon_ceq (const char c1, const char c2, bool case_sensitive)
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
    return true;
  return false;
}

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
  bacon_foutc (stderr, '\n');
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
  bacon_foutc (stderr, '\n');
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
  bacon_foutc (stderr, '\n');
}

void
bacon_foutc (FILE *stream, const char c)
{
  fputc (c, stream);
}

void
bacon_outc (const char c)
{
  bacon_foutc (stdout, c);
}

void
bacon_foutcco (FILE *stream, int color, bool bold, const char c)
{
  bacon_color (color, bold, stream);
  bacon_foutc (stream, c);
  bacon_no_color (stream);
}

void
bacon_outcco (int color, bool bold, const char c)
{
  bacon_color (color, bold, stdout);
  bacon_outc (c);
  bacon_no_color (stdout);
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
  bacon_foutc (stream, '\n');
}

void
bacon_foutco (FILE *stream, int color, bool bold, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    bacon_color (color, bold, stream);
    vfprintf (stream, msg, a);
    bacon_no_color (stream);
    va_end (a);
  }
}

void
bacon_foutlnco (FILE *stream, int color, bool bold, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    bacon_color (color, bold, stream);
    vfprintf (stream, msg, a);
    bacon_no_color (stream);
    va_end (a);
  }
  bacon_foutc (stream, '\n');
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
  bacon_outc ('\n');
}

void
bacon_outco (int color, bool bold, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    bacon_color (color, bold, stdout);
    vfprintf (stdout, msg, a);
    bacon_no_color (stdout);
    va_end (a);
  }
}

void
bacon_outlnco (int color, bool bold, const char *msg, ...)
{
  va_list a;

  if (msg && *msg) {
    va_start (a, msg);
    bacon_color (color, bold, stdout);
    vfprintf (stdout, msg, a);
    bacon_no_color (stdout);
    va_end (a);
  }
  bacon_outc ('\n');
}

void
bacon_outi (const int level, const char *msg, ...)
{
  int i;
  int j;
  va_list a;

  for (i = 0; i < level; ++i)
    for (j = 0; j < BACON_INDENT_SIZE; ++j)
      bacon_outc (' ');

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
}

void
bacon_outlni (const int level, const char *msg, ...)
{
  int i;
  int j;
  va_list a;

  for (i = 0; i < level; ++i)
    for (j = 0; j < BACON_INDENT_SIZE; ++j)
      bacon_outc (' ');

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  bacon_outc ('\n');
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
  bacon_outc ('\n');
}

void
bacon_color (int color, bool bold, FILE *stream)
{
  if (!g_use_color)
    return;

  if (bold)
    fputs (BACON_COLOR_CODE_BOLD, stream);

  switch (color) {
  case BACON_COLOR_BLACK:
    fputs (BACON_COLOR_CODE_BLACK, stream);
    break;
  case BACON_COLOR_RED:
    fputs (BACON_COLOR_CODE_RED, stream);
    break;
  case BACON_COLOR_GREEN:
    fputs (BACON_COLOR_CODE_GREEN, stream);
    break;
  case BACON_COLOR_YELLOW:
    fputs (BACON_COLOR_CODE_YELLOW, stream);
    break;
  case BACON_COLOR_BLUE:
    fputs (BACON_COLOR_CODE_BLUE, stream);
    break;
  case BACON_COLOR_MAGENTA:
    fputs (BACON_COLOR_CODE_MAGENTA, stream);
    break;
  case BACON_COLOR_CYAN:
    fputs (BACON_COLOR_CODE_CYAN, stream);
    break;
  case BACON_COLOR_WHITE:
    fputs (BACON_COLOR_CODE_WHITE, stream);
    break;
  default:
    ;
  }
}

void
bacon_no_color (FILE *stream)
{
  if (!g_use_color)
    return;
  fputs (BACON_COLOR_CODE_NONE, stream);
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

char
bacon_tolower (char c)
{
  switch (c) {
  case 'A':
    return 'a';
  case 'B':
    return 'b';
  case 'C':
    return 'c';
  case 'D':
    return 'd';
  case 'E':
    return 'e';
  case 'F':
    return 'f';
  case 'G':
    return 'g';
  case 'H':
    return 'h';
  case 'I':
    return 'i';
  case 'J':
    return 'j';
  case 'K':
    return 'k';
  case 'L':
    return 'l';
  case 'M':
    return 'm';
  case 'N':
    return 'n';
  case 'O':
    return 'o';
  case 'P':
    return 'p';
  case 'Q':
    return 'q';
  case 'R':
    return 'r';
  case 'S':
    return 's';
  case 'T':
    return 't';
  case 'U':
    return 'u';
  case 'V':
    return 'v';
  case 'W':
    return 'w';
  case 'X':
    return 'x';
  case 'Y':
    return 'y';
  case 'Z':
    return 'z';
  default:
    ;
  }
  return c;
}

bool
bacon_isdigit (char c)
{
  switch (c) {
  case '0': case '1': case '2': case '3': case '4': case '5': case '7':
  case '8': case '9':
    return true;
  default:
    ;
  }
  return false;
}

bool
bacon_isspace (char c)
{
  switch (c) {
  case ' ': case '\f': case '\n': case '\r': case '\t': case '\v':
    return true;
  default:
    ;
  }
  return false;
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
bacon_strfposof (const char *str, const char *query, bool case_sensitive)
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
bacon_stroccur (const char *str, const char *query, bool case_sensitive)
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

BaconQueryTokenList *
bacon_query_token_list_new (const char *query)
{
  size_t n;
  size_t token_pos;
  size_t query_pos;
  BaconQueryTokenList *p;
  BaconQueryTokenList *list;

  list = NULL;
  if (query && *query) {
    query_pos = 0;
    n = strlen (query);
    char buffer[n + 1];
    strncpy (buffer, query, n);
    buffer[n] = '\0';
    while (bacon_isspace (buffer[n - 1]))
      buffer[n-- - 1] = '\0';
    while (true) {
      while (buffer[query_pos] && bacon_isspace (buffer[query_pos]))
        query_pos++;
      if (buffer[query_pos]) {
        if (!list) {
          p = bacon_new (BaconQueryTokenList);
          p->prev = NULL;
        } else {
          for (p = list; p; p = p->next)
            if (!p->next)
              break;
          p->next = bacon_new (BaconQueryTokenList);
          p->next->prev = p;
          p = p->next;
        }
        p->next = NULL;
        token_pos = 0;
        while (buffer[query_pos]) {
          p->token[token_pos++] = bacon_tolower (buffer[query_pos++]);
          if ((token_pos == (BACON_TOKEN_MAX - 1)) ||
              bacon_isspace (buffer[query_pos]))
            break;
        }
        p->token[token_pos] = '\0';
        for (; p; p = p->prev)
          if (!p->prev)
            break;
        list = p;
        if (!buffer[query_pos])
          break;
      }
    }
  }
  return list;
}

void
bacon_query_token_list_free (BaconQueryTokenList *query_tokens)
{
  for (; query_tokens; query_tokens = query_tokens->next) {
    bacon_free (query_tokens->prev);
    if (!query_tokens->next)
      break;
  }
}

bool
bacon_search (const char *content, BaconQueryTokenList *query_tokens)
{
  size_t n_content;
  bool match;
  BaconQueryTokenList *p;

  match = false;
  if (query_tokens) {
    n_content = strlen (content);
    if (n_content) {
      char buffer[n_content + 1];
      bacon_strtolower (buffer, n_content, content);
      for (p = query_tokens; p; p = p->next) {
        if (strstr (buffer, p->token))
          match = true;
        else {
          match = false;
          break;
        }
        if (!p->next)
          break;
      }
    }
  }
  return match;
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

