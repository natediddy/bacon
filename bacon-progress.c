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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-os.h"
#include "bacon-out.h"
#include "bacon-progress.h"
#include "bacon-str.h"
#include "bacon-util.h"

#ifdef BACON_OS_UNIX
# include <sys/ioctl.h>
# include <unistd.h>
#endif

#ifdef BACON_OS_WINDOWS
# include <windows.h>
#endif

#define BACON_FALLBACK_CONSOLE_WIDTH 40
#define BACON_OUTPUT_MAX             1024
#define BACON_PERCENT_MAX            5
#define BACON_SIZE_MAX               10
#define BACON_ETA_MAX                14
#define BACON_SPEED_MAX              BACON_SIZE_MAX + 2

#define BACON_PERCENT_DEFAULT        "0%"
#define BACON_ETA_DEFAULT            "0s"
#define BACON_SPEED_SUFFIX           "/s"
#define BACON_BAR_START              '['
#define BACON_BAR_END                ']'
#define BACON_BAR_HAS                '#'
#define BACON_BAR_NOT                '-'

#define BACON_BAR_END_POS             \
  (s_console_width                  - \
   ((int) strlen (s_current_buffer) + \
    (int) strlen (s_total_buffer)   + \
    (int) strlen (s_speed_buffer)   + \
    (int) strlen (s_eta_buffer)     + \
    (int) strlen (s_percent_buffer)))

#define BACON_PROPELLER_0            '|'
#define BACON_PROPELLER_1            '/'
#define BACON_PROPELLER_2            '-'
#define BACON_PROPELLER_3            '\\'
#define BACON_PROPELLER_SIZE         4
#define BACON_FILE_WAIT(m)           (m >= BACON_SEC_MILLIS)
#define BACON_PAGE_WAIT(m)           (m >= (BACON_SEC_MILLIS / 25))
#define BACON_DAY_SECS               86400
#define BACON_HOUR_SECS              3600
#define BACON_MIN_SECS               60
#define BACON_MOD_SEC                60

static int        s_console_width  = BACON_FALLBACK_CONSOLE_WIDTH;
static int        s_output_pos     = 0;
static int        s_output_rem     = 0;
static long       s_per_sec        = -1;
static time_t     s_start_time     = -1;
static time_t     s_end_time       = -1;
static size_t     s_propeller_pos  = 0;
static char       s_output_buffer  [BACON_OUTPUT_MAX];
static char       s_current_buffer [BACON_SIZE_MAX];
static char       s_total_buffer   [BACON_SIZE_MAX];
static char       s_percent_buffer [BACON_PERCENT_MAX];
static char       s_eta_buffer     [BACON_ETA_MAX];
static char       s_speed_buffer   [BACON_SPEED_MAX];
static const char s_propeller      [BACON_PROPELLER_SIZE] = {
  BACON_PROPELLER_0,
  BACON_PROPELLER_1,
  BACON_PROPELLER_2,
  BACON_PROPELLER_3
};

static void
bacon_output_init (void)
{
  int width;

  width = -1;

#ifdef BACON_OS_UNIX
  struct winsize x;

  if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &x) != -1)
    width = x.ws_col;
#endif

#ifdef BACON_OS_WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO x;

  if (GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE), &x))
    width = x.dwSize.X;
#endif

  if (width == -1)
    s_console_width = BACON_FALLBACK_CONSOLE_WIDTH;
  else
    s_console_width = width;

  s_output_pos = 0;
  s_output_rem = s_console_width;

  if (*s_current_buffer)
    *s_current_buffer = '\0';

  if (*s_total_buffer)
    *s_total_buffer = '\0';

  if (*s_percent_buffer)
    *s_percent_buffer = '\0';

  if (*s_eta_buffer)
    *s_eta_buffer = '\0';

  if (*s_speed_buffer)
    *s_speed_buffer = '\0';
}

static void
bacon_output_add_string (const char *s)
{
  size_t n;

  n = strlen (s);
  strncpy (s_output_buffer + s_output_pos, s, n);
  s_output_pos += n;
  s_output_rem -= n;
}

static void
bacon_output_add_char (char c)
{
  s_output_buffer[s_output_pos++] = c;
  s_output_rem--;
}

static void
bacon_set_bytes_per_sec (double current)
{
  if (current <= 0.0) {
    s_per_sec = -1;
    return;
  }
  s_per_sec = (current / (s_start_time - time (NULL)));
  if (s_per_sec < 0)
    s_per_sec = -s_per_sec;
}

static void
bacon_format_percent (double x)
{
  if (bacon_nan_value (x) || bacon_nan_value (x * 100)) {
    strncpy (s_percent_buffer, BACON_PERCENT_DEFAULT, BACON_PERCENT_MAX);
    return;
  }
  snprintf (s_percent_buffer, BACON_PERCENT_MAX, "%.0f%%", (x * 100));
}

static void
bacon_format_total (unsigned long bytes)
{
  bacon_strbytes (s_total_buffer, BACON_SIZE_MAX, bytes);
}

static void
bacon_format_current (unsigned long bytes)
{
  bacon_strbytes (s_current_buffer, BACON_SIZE_MAX, bytes);
}

static void
bacon_format_speed (void)
{
  if (s_per_sec < 0) {
    snprintf (s_speed_buffer, BACON_SIZE_MAX + strlen (BACON_SPEED_SUFFIX),
              "0%s", BACON_SPEED_SUFFIX);
    return;
  }
  bacon_strbytes (s_speed_buffer, BACON_SIZE_MAX, s_per_sec);
  strncpy (s_speed_buffer + strlen (s_speed_buffer),
           BACON_SPEED_SUFFIX, strlen (BACON_SPEED_SUFFIX) + 1);
}

static void
bacon_format_eta (double x)
{
  int d;
  int h;
  int m;
  int s;
  size_t n;

  if (x <= 0.0) {
    strncpy (s_eta_buffer, BACON_ETA_DEFAULT, strlen (BACON_ETA_DEFAULT) + 1);
    return;
  }

  d = ((x / s_per_sec) / BACON_DAY_SECS);
  h = ((x / s_per_sec) / BACON_HOUR_SECS);
  m = ((x / s_per_sec) / BACON_MIN_SECS);
  s = ((int) (x / s_per_sec) % BACON_MOD_SEC);
  n = 0;

  if (d > 0) {
    snprintf (s_eta_buffer, BACON_ETA_MAX, "%id", d);
    n = strlen (s_eta_buffer);
  }

  if (h > 0) {
    snprintf (s_eta_buffer + n, BACON_ETA_MAX - n, "%ih", h);
    n = strlen (s_eta_buffer);
  }

  if (m > 0) {
    snprintf (s_eta_buffer + n, BACON_ETA_MAX - n, "%im", m);
    n = strlen (s_eta_buffer);
  }

  if (s > 0)
    snprintf (s_eta_buffer + n, BACON_ETA_MAX - n, "%is", s);
}

static void
bacon_output_finish (void)
{
  while (s_output_rem > 2) {
    s_output_buffer[s_output_pos++] = ' ';
    s_output_rem--;
  }
  s_output_buffer[s_output_pos++] = '\r';
  s_output_buffer[s_output_pos] = '\0';
}

static void
bacon_output_display (void)
{
  bacon_out (s_output_buffer);
  fflush (stdout);
}

void
bacon_progress_init (void)
{
  s_start_time = time (NULL);
}

void
bacon_progress_deinit (bool newline)
{
  s_end_time = time (NULL);
  if (newline)
    bacon_outc ('\n');
}

void
bacon_progress_file (double total, double current)
{
  static struct timeval last = { -1, -1 };

  int i;
  int has_stop_pos;
  int bar_end_pos;
  long millis;
  double x;
  struct timeval now;

  if ((last.tv_sec == -1) && (last.tv_usec == -1))
    millis = -1;
  else {
    bacon_get_time_of_day (&now);
    millis = bacon_get_millis (&last, &now);
  }

  if ((millis == -1) || BACON_FILE_WAIT (millis)) {
    bacon_output_init ();
    bacon_format_current ((unsigned long) current);
    bacon_format_total ((unsigned long) total);
    x = (total - current);
    bacon_format_eta (x);
    x = (current / total);
    bacon_format_percent (x);
    bacon_set_bytes_per_sec (current);
    bacon_format_speed ();
    bacon_output_add_string (s_current_buffer);
    bacon_output_add_char ('/');
    bacon_output_add_string (s_total_buffer);
    bacon_output_add_char (' ');
    bar_end_pos = BACON_BAR_END_POS - 8;
    if (bar_end_pos) {
      bacon_output_add_char (BACON_BAR_START);
      has_stop_pos = bacon_round (x * bar_end_pos);
      for (i = 0; i < has_stop_pos; ++i)
        bacon_output_add_char (BACON_BAR_HAS);
      for (; i < bar_end_pos; ++i)
        bacon_output_add_char (BACON_BAR_NOT);
      bacon_output_add_char (BACON_BAR_END);
    }
    bacon_output_add_char (' ');
    bacon_output_add_string (s_speed_buffer);
    bacon_output_add_char (' ');
    bacon_output_add_string (s_eta_buffer);
    bacon_output_add_char (' ');
    bacon_output_add_string (s_percent_buffer);
    bacon_output_finish ();
    bacon_output_display ();
    if (millis == -1)
      bacon_get_time_of_day (&last);
    else {
      last.tv_sec = now.tv_sec;
      last.tv_usec = now.tv_usec;
    }
  }
}

void
bacon_progress_page (double total, double current)
{
  static struct timeval last = { -1, -1 };

  long millis;
  struct timeval now;

  if ((last.tv_sec == -1) && (last.tv_usec == -1))
    millis = -1;
  else {
    bacon_get_time_of_day (&now);
    millis = bacon_get_millis (&last, &now);
  }

  if ((millis == -1) || BACON_PAGE_WAIT (millis)) {
    bacon_output_init ();
    bacon_output_add_string ("Loading... ");
    if (s_propeller_pos == BACON_PROPELLER_SIZE)
      s_propeller_pos = 0;
    bacon_output_add_char (s_propeller[s_propeller_pos++]);
    bacon_output_finish ();
    bacon_output_display ();
    if (millis == -1)
      bacon_get_time_of_day (&last);
    else {
      last.tv_sec = now.tv_sec;
      last.tv_usec = now.tv_usec;
    }
  }
}

