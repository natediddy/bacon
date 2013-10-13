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

#include "bacon-out.h"
#include "bacon-util.h"

void *
bacon_malloc (size_t n)
{
  void *ptr;

  ptr = malloc (n);
  if (!ptr) {
    bacon_error ("malloc failed: %s", strerror (errno));
    exit (EXIT_FAILURE);
  }
  return ptr;
}

void *
bacon_realloc (void *ptr, size_t n)
{
  void *pptr;

  pptr = realloc (ptr, n);
  if (!pptr) {
    bacon_error ("realloc failed: %s", strerror (errno));
    exit (EXIT_FAILURE);
  }
  return pptr;
}

bool
bacon_nan_value (double v)
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
bacon_get_millis (const struct timeval *s, const struct timeval *e)
{
  long sec;
  long usec;

  sec = ((e->tv_sec - s->tv_sec) * BACON_SEC_MILLIS);
  usec = ((e->tv_usec - s->tv_usec) / BACON_SEC_MILLIS);
  return (sec + usec);
}

