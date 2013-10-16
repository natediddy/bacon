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

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "bacon-colors.h"
#include "bacon-out.h"

#define BACON_INDENT_SIZE      4
#define BACON_COLOR_STRING_MAX 128

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

extern char *       g_program_name;
extern BaconBoolean g_use_color;
extern BaconBoolean g_in_progress;

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
    bacon_foutco (stderr, BACON_PROGRAM_NAME_COLOR,
                  BACON_TRUE, "%s:", BACON_PRINT_PROGRAM_NAME);
    if (file && *file)
      bacon_foutco (stderr, BACON_DEBUG_FILE_TAG_COLOR,
                    BACON_TRUE, "%s:", file);
    if (func && *func)
      bacon_foutco (stderr, BACON_DEBUG_FUNC_TAG_COLOR,
                    BACON_TRUE, "%s:", func);
    if (line >= 0)
      bacon_foutco (stderr, BACON_DEBUG_LINE_TAG_COLOR,
                    BACON_TRUE, "%i:", line);
    bacon_foutco (stderr, BACON_DEBUG_TAG_COLOR,
                  BACON_TRUE, BACON_DEBUG_TAG);
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
    bacon_foutco (stderr, BACON_PROGRAM_NAME_COLOR,
                  BACON_TRUE, "%s: ", BACON_PRINT_PROGRAM_NAME);
    bacon_foutco (stderr, BACON_ERROR_TAG_COLOR, BACON_TRUE, BACON_ERROR_TAG);
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
    bacon_foutco (stderr, BACON_PROGRAM_NAME_COLOR,
                  BACON_TRUE, "%s: ", BACON_PRINT_PROGRAM_NAME);
    bacon_foutco (stderr, BACON_ERROR_TAG_COLOR, BACON_TRUE, BACON_WARN_TAG);
    va_start (a, msg);
    vfprintf (stderr, msg, a);
    va_end (a);
  }
  bacon_foutc (stderr, '\n');
}

void
bacon_foutc (FILE *stream, char c)
{
#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif
  fputc (c, stream);
}

void
bacon_outc (const char c)
{
  bacon_foutc (stdout, c);
}

void
bacon_foutcco (FILE *stream, int color, BaconBoolean bold, char c)
{
#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif
  bacon_color (color, bold, stream);
  bacon_foutc (stream, c);
  bacon_no_color (stream);
}

void
bacon_outcco (int color, BaconBoolean bold, char c)
{
  bacon_color (color, bold, stdout);
  bacon_outc (c);
  bacon_no_color (stdout);
}

void
bacon_fout (FILE *stream, const char *msg, ...)
{
  va_list a;

#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif

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

#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif

  if (msg && *msg) {
    va_start (a, msg);
    vfprintf (stream, msg, a);
    va_end (a);
  }
  bacon_foutc (stream, '\n');
}

void
bacon_foutco (FILE *stream,
              int color,
              BaconBoolean bold,
              const char *msg,
              ...)
{
  va_list a;

#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif

  if (msg && *msg) {
    va_start (a, msg);
    bacon_color (color, bold, stream);
    vfprintf (stream, msg, a);
    bacon_no_color (stream);
    va_end (a);
  }
}

void
bacon_foutlnco (FILE *stream,
                int color,
                BaconBoolean bold,
                const char *msg,
                ...)
{
  va_list a;

#ifdef HAVE_ISATTY
  if (g_in_progress && !isatty (fileno (stream)))
    return;
#endif

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
bacon_outco (int color, BaconBoolean bold, const char *msg, ...)
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
bacon_outlnco (int color, BaconBoolean bold, const char *msg, ...)
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
    bacon_foutco (stdout, BACON_PROGRAM_NAME_COLOR,
                  BACON_TRUE, "%s: ", BACON_PRINT_PROGRAM_NAME);
    va_start (a, msg);
    vfprintf (stdout, msg, a);
    va_end (a);
  }
  bacon_outc ('\n');
}

void
bacon_color (int color, BaconBoolean bold, FILE *stream)
{
  if (!g_use_color)
    return;

  if (bold)
    bacon_fout (stream, BACON_COLOR_CODE_BOLD);

  switch (color) {
  case BACON_COLOR_BLACK:
    bacon_fout (stream, BACON_COLOR_CODE_BLACK);
    break;
  case BACON_COLOR_RED:
    bacon_fout (stream, BACON_COLOR_CODE_RED);
    break;
  case BACON_COLOR_GREEN:
    bacon_fout (stream, BACON_COLOR_CODE_GREEN);
    break;
  case BACON_COLOR_YELLOW:
    bacon_fout (stream, BACON_COLOR_CODE_YELLOW);
    break;
  case BACON_COLOR_BLUE:
    bacon_fout (stream, BACON_COLOR_CODE_BLUE);
    break;
  case BACON_COLOR_MAGENTA:
    bacon_fout (stream, BACON_COLOR_CODE_MAGENTA);
    break;
  case BACON_COLOR_CYAN:
    bacon_fout (stream, BACON_COLOR_CODE_CYAN);
    break;
  case BACON_COLOR_WHITE:
    bacon_fout (stream, BACON_COLOR_CODE_WHITE);
    break;
  default:
    ;
  }
}

void
bacon_no_color (FILE *stream)
{
  if (g_use_color)
    bacon_fout (stream, BACON_COLOR_CODE_NONE);
}

