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

#include "bacon.h"

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_SEC_MILLIS 1000

#define bacon_new(type)        ((type *) bacon_malloc (sizeof (type)))
#define bacon_newa(type, size) ((type *) bacon_malloc (size))
#define bacon_free(p) \
  do {                \
    if (!(p))         \
      break;          \
    free ((p));       \
    (p) = NULL;       \
  } while (BACON_FALSE)

#define bacon_round(x) \
  (((x) >= 0) ? ((int) ((x) + 0.5)) : ((int) ((x) - 0.5)))

#define bacon_list_append(type, root, p) \
  do { \
    if (!root) { \
      p = bacon_new (type); \
      p->prev = NULL; \
    } else { \
      for (p = root; p; p = p->next) \
        if (!p->next) \
          break; \
      p->next = bacon_new (type); \
      p->next->prev = p; \
      p = p->next; \
    } \
    p->next = NULL; \
  } while (BACON_FALSE)

#define bacon_list_rewind(root, p) \
  do { \
    for (; p; p = p->prev) \
      if (!p->prev) \
        break; \
    root = p; \
  } while (BACON_FALSE)

#define bacon_list_free(root) \
  do { \
    for (; root; root = root->next) { \
      bacon_free (root->prev); \
      if (!root->next) \
        break; \
    } \
    bacon_free (root); \
  } while (BACON_FALSE)

void *bacon_malloc (size_t n);
void *bacon_realloc (void *ptr, size_t n);
BaconBoolean bacon_nan_value (double v);
int bacon_ndigits (int v);
#ifdef HAVE_SYS_TIME_H
void bacon_get_time_of_day (struct timeval *tv);
long bacon_get_millis (const struct timeval *s, const struct timeval *e);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BACON_UTIL_H */

