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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-os.h"
#include "bacon-progress.h"
#include "bacon-util.h"

#ifdef BACON_OS_UNIX
# include <sys/ioctl.h>
# include <unistd.h>
#endif

#ifdef BACON_OS_WINDOWS
# include <windows.h>
#endif

#define BACON_FALLBACK_CONSOLE_WIDTH 40

#define BACON_OUTPUT_MAX  1024
#define BACON_PERCENT_MAX 5
#define BACON_SIZE_MAX    10
#define BACON_ETA_MAX     14
#define BACON_SPEED_MAX   BACON_SIZE_MAX + 2

#define BACON_PERCENT_DEFAULT "0%"
#define BACON_ETA_DEFAULT     "0s"
#define BACON_SPEED_SUFFIX    "/s"

#define BACON_BAR_START '['
#define BACON_BAR_END   ']'
#define BACON_BAR_HAS   '#'
#define BACON_BAR_NOT   '-'

#define BACON_BAR_END_POS \
  (console_width                  - \
   ((int) strlen (current_buffer) + \
    (int) strlen (total_buffer)   + \
    (int) strlen (speed_buffer)   + \
    (int) strlen (eta_buffer)     + \
    (int) strlen (percent_buffer)))

#define BACON_PROPELLER_0 '|'
#define BACON_PROPELLER_1 '/'
#define BACON_PROPELLER_2 '-'
#define BACON_PROPELLER_3 '\\'

#define BACON_PROPELLER_SIZE 4

#define BACON_FILE_WAIT(m) (m >= BACON_SEC_MILLIS)
#define BACON_PAGE_WAIT(m) (m >= (BACON_SEC_MILLIS / 25))

#define BACON_DAY_SECS  86400
#define BACON_HOUR_SECS 3600
#define BACON_MIN_SECS  60
#define BACON_MOD_SEC   60

static int    console_width  = BACON_FALLBACK_CONSOLE_WIDTH;
static int    output_pos     = 0;
static int    output_rem     = 0;
static long   per_sec        = -1;
static time_t start_time     = -1;
static time_t end_time       = -1;
static size_t propeller_pos  = 0;

static char   output_buffer  [BACON_OUTPUT_MAX];
static char   current_buffer [BACON_SIZE_MAX];
static char   total_buffer   [BACON_SIZE_MAX];
static char   percent_buffer [BACON_PERCENT_MAX];
static char   eta_buffer     [BACON_ETA_MAX];
static char   speed_buffer   [BACON_SPEED_MAX];

static const char propeller[BACON_PROPELLER_SIZE] =
{
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
    console_width = BACON_FALLBACK_CONSOLE_WIDTH;
  else
    console_width = width;

  output_pos = 0;
  output_rem = console_width;

  if (*current_buffer)
    *current_buffer = '\0';

  if (*total_buffer)
    *total_buffer = '\0';

  if (*percent_buffer)
    *percent_buffer = '\0';

  if (*eta_buffer)
    *eta_buffer = '\0';

  if (*speed_buffer)
    *speed_buffer = '\0';
}

static void
bacon_output_add_string (const char *s)
{
  size_t n;

  n = strlen (s);
  strncpy (output_buffer + output_pos, s, n);
  output_pos += n;
  output_rem -= n;
}

static void
bacon_output_add_char (const char c)
{
  output_buffer[output_pos++] = c;
  output_rem--;
}

static void
bacon_set_bytes_per_sec (double current)
{
  if (current <= 0.0)
  {
    per_sec = -1;
    return;
  }
  per_sec = ((int) current / ((int) start_time - (int) time (NULL)));
  if (per_sec < 0)
    per_sec = -per_sec;
}

static void
bacon_format_percent (double x)
{
  if (bacon_nan_value (x) || bacon_nan_value (x * 100))
  {
    strncpy (percent_buffer, BACON_PERCENT_DEFAULT, BACON_PERCENT_MAX);
    return;
  }
  snprintf (percent_buffer, BACON_PERCENT_MAX, "%.0f%%", (x * 100));
}

static void
bacon_format_total (CmByte bytes)
{
  bacon_byte_str_format (total_buffer, BACON_SIZE_MAX, bytes);
}

static void
bacon_format_current (CmByte bytes)
{
  bacon_byte_str_format (current_buffer, BACON_SIZE_MAX, bytes);
}

static void
bacon_format_speed (void)
{
  if (per_sec < 0)
  {
    snprintf (speed_buffer, BACON_SIZE_MAX + strlen (BACON_SPEED_SUFFIX),
        "0%c%s", BACON_BYTE_SYMBOL, BACON_SPEED_SUFFIX);
    return;
  }
  bacon_byte_str_format (speed_buffer, BACON_SIZE_MAX, (CmByte) per_sec);
  strncpy (speed_buffer + strlen (speed_buffer),
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

  if (x <= 0.0)
  {
    strncpy (eta_buffer, BACON_ETA_DEFAULT, strlen (BACON_ETA_DEFAULT) + 1);
    return;
  }

  d = (((int) x / per_sec) / BACON_DAY_SECS);
  h = (((int) x / per_sec) / BACON_HOUR_SECS);
  m = (((int) x / per_sec) / BACON_MIN_SECS);
  s = (((int) x / per_sec) % BACON_MOD_SEC);
  n = 0;

  if (d > 0)
  {
    snprintf (eta_buffer, BACON_ETA_MAX, "%id", d);
    n = strlen (eta_buffer);
  }

  if (h > 0)
  {
    snprintf (eta_buffer + n, BACON_ETA_MAX - n, "%ih", h);
    n = strlen (eta_buffer);
  }

  if (m > 0)
  {
    snprintf (eta_buffer + n, BACON_ETA_MAX - n, "%im", m);
    n = strlen (eta_buffer);
  }

  if (s > 0)
    snprintf (eta_buffer + n, BACON_ETA_MAX - n, "%is", s);
}

static void
bacon_output_finish (void)
{
  while (output_rem > 2)
  {
    output_buffer[output_pos++] = ' ';
    output_rem--;
  }
  output_buffer[output_pos++] = '\r';
  output_buffer[output_pos] = '\0';
}

static void
bacon_output_display (void)
{
  bacon_out (output_buffer);
  fflush (stdout);
}

void
bacon_progress_init (void)
{
  start_time = time (NULL);
}

void
bacon_progress_deinit (bool newline)
{
  end_time = time (NULL);
  if (newline)
    bacon_outln(NULL);
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
  else
  {
    bacon_get_time_of_day (&now);
    millis = bacon_get_millis (&last, &now);
  }

  if ((millis == -1) || BACON_FILE_WAIT (millis))
  {
    bacon_output_init ();
    bacon_format_current ((CmByte ) current);
    bacon_format_total ((CmByte ) total);
    x = (total - current);
    bacon_format_eta (x);
    x = (current / total);
    bacon_format_percent (x);
    bacon_set_bytes_per_sec (current);
    bacon_format_speed ();
    bacon_output_add_string (current_buffer);
    bacon_output_add_char ('/');
    bacon_output_add_string (total_buffer);
    bacon_output_add_char (' ');
    bar_end_pos = BACON_BAR_END_POS - 8;
    if (bar_end_pos)
    {
      bacon_output_add_char (BACON_BAR_START);
      has_stop_pos = bacon_round (x * bar_end_pos);
      for (i = 0; i < has_stop_pos; ++i)
        bacon_output_add_char (BACON_BAR_HAS);
      for (; i < bar_end_pos; ++i)
        bacon_output_add_char (BACON_BAR_NOT);
      bacon_output_add_char (BACON_BAR_END);
    }
    bacon_output_add_char (' ');
    bacon_output_add_string (speed_buffer);
    bacon_output_add_char (' ');
    bacon_output_add_string (eta_buffer);
    bacon_output_add_char (' ');
    bacon_output_add_string (percent_buffer);
    bacon_output_finish ();
    bacon_output_display ();
    if (millis == -1)
      bacon_get_time_of_day (&last);
    else
    {
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
  else
  {
    bacon_get_time_of_day (&now);
    millis = bacon_get_millis (&last, &now);
  }

  if ((millis == -1) || BACON_PAGE_WAIT (millis))
  {
    bacon_output_init ();
    bacon_output_add_string ("Loading... ");
    if (propeller_pos == BACON_PROPELLER_SIZE)
      propeller_pos = 0;
    bacon_output_add_char(propeller[propeller_pos++]);
    bacon_output_finish ();
    bacon_output_display ();
    if (millis == -1)
      bacon_get_time_of_day (&last);
    else
    {
      last.tv_sec = now.tv_sec;
      last.tv_usec = now.tv_usec;
    }
  }
}

