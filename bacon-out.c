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
#include "bacon-colors.h"
#include "bacon-out.h"

#define BACON_INDENT_SIZE      4
#define BACON_COLOR_STRING_MAX 128

#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG  "DEBUG"
#endif
#define BACON_ERROR_TAG   "error"
#define BACON_WARNING_TAG "warning"
#define BACON_NORMAL_TAG  ""

extern char *       g_program_name;
extern BaconBoolean g_use_color;

#ifdef BACON_DEBUG
void
__bacon_debug (const char *file,
               const char *func,
               int line,
               const char *msg,
               ...)
{
  va_list a;

  if (msg && *msg) {
    bacon_fout (stderr, "%s:%s:%s:%s:%s: ",
                BACON_COLOR_S (BACON_PROGRAM_NAME_COLOR,
                               BACON_PRINT_PROGRAM_NAME),
                BACON_COLOR_S (BACON_DEBUG_TAG_COLOR, BACON_DEBUG_TAG),
                BACON_COLOR_S (BACON_DEBUG_FILE_TAG_COLOR, file),
                BACON_COLOR_S (BACON_DEBUG_FUNC_TAG_COLOR, func),
                BACON_COLOR_I (BACON_DEBUG_LINE_TAG_COLOR, line));
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
    bacon_fout (stderr, "%s: %s: ",
                BACON_COLOR_S (BACON_PROGRAM_NAME_COLOR,
                               BACON_PRINT_PROGRAM_NAME),
                BACON_COLOR_S (BACON_ERROR_TAG_COLOR, BACON_ERROR_TAG));
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
    bacon_fout (stderr, "%s: %s: ",
                BACON_COLOR_S (BACON_PROGRAM_NAME_COLOR,
                               BACON_PRINT_PROGRAM_NAME),
                BACON_COLOR_S (BACON_WARNING_TAG_COLOR, BACON_WARNING_TAG));
    va_start (a, msg);
    vfprintf (stderr, msg, a);
    va_end (a);
  }
  bacon_foutc (stderr, '\n');
}

void
bacon_foutc (FILE *stream, char c)
{
  fputc (c, stream);
}

void
bacon_outc (const char c)
{
  bacon_foutc (stdout, c);
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
bacon_outi (int level, const char *msg, ...)
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
bacon_outlni (int level, const char *msg, ...)
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
    bacon_fout (stdout, "%s: ",
                BACON_COLOR_S (BACON_PROGRAM_NAME_COLOR,
                               BACON_PRINT_PROGRAM_NAME));
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  bacon_outc ('\n');
}

