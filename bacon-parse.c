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

#include "bacon-net.h"
#include "bacon-out.h"
#include "bacon-parse.h"
#include "bacon-util.h"

#define BACON_CODENAME_TAG       "<span class=\"codename\">"
#define BACON_FULLNAME_TAG       "<span class=\"fullname\">"
#define BACON_ROM_NAME_PATTERN   ".zip\">"
#define BACON_HASH_PATTERN       "md5sum: "
#define BACON_GET_PATTERN        BACON_GET_CM_URL "/"
#define BACON_SIZE_TAG           "<td>"
#define BACON_DATE_TAG           "<td>"
#ifdef BACON_GTK
# define BACON_THUMB_URL_PATTERN "wiki.cyanogenmod.org/images/"
#endif

#define BACON_LINE_MAX 1024

#define bacon_find_and_fill(__dst, __src, __p, __np, __x, __c) \
  do {                                                         \
    __x = strstr (__src, __p);                                 \
    if (__x && *__x) {                                         \
      __x = __x + __np;                                        \
      bacon_fill_buffer (__dst, __x, __c);                     \
      __src = __x;                                             \
    }                                                          \
  } while (BACON_FALSE)

static size_t s_n_codename_tag      = 0;
static size_t s_n_fullname_tag      = 0;
static size_t s_n_rom_name_pattern  = 0;
static size_t s_n_hash_pattern      = 0;
static size_t s_n_get_pattern       = 0;
static size_t s_n_size_tag          = 0;
static size_t s_n_date_tag          = 0;
#ifdef BACON_GTK
static size_t s_n_thumb_url_pattern = 0;
#endif

static void
bacon_set_size_values (void)
{
  if (!s_n_codename_tag)
    s_n_codename_tag = strlen (BACON_CODENAME_TAG);

  if (!s_n_fullname_tag)
    s_n_fullname_tag = strlen (BACON_FULLNAME_TAG);

  if (!s_n_rom_name_pattern)
    s_n_rom_name_pattern = strlen (BACON_ROM_NAME_PATTERN);

  if (!s_n_hash_pattern)
    s_n_hash_pattern = strlen (BACON_HASH_PATTERN);

  if (!s_n_get_pattern)
    s_n_get_pattern = strlen (BACON_GET_PATTERN);

  if (!s_n_size_tag)
    s_n_size_tag = strlen (BACON_SIZE_TAG);

  if (!s_n_date_tag)
    s_n_date_tag = strlen (BACON_DATE_TAG);

#ifdef BACON_GTK
  if (!s_n_thumb_url_pattern)
    s_n_thumb_url_pattern = strlen (BACON_THUMB_URL_PATTERN);
#endif
}

static void
bacon_get_line (char *line, const char *data, size_t *pos)
{
  size_t x;
  size_t p;

  p = *pos;
  if (data[p] && (data[p] == '\n'))
    ++p;

  x = 0;
  for (; data[p] && (data[p] != '\n'); ++p)
    line[x++] = data[p];
  line[x] = '\0';
  *pos = p;
}

static void
bacon_fill_buffer_pos (char *dst,
                       const char *src,
                       size_t *pos,
                       const char stop)
{
  size_t x;
  size_t p;

  x = 0;
  p = *pos;

  for (; src[p] != stop; ++p)
    dst[x++] = src[p];

  dst[x] = '\0';
  if ((src[p] == stop) && (stop != '\0'))
    ++p;
  *pos = p;
}

static void
bacon_fill_buffer (char *dst, const char *src, const char stop)
{
  size_t x;
  size_t p;

  x = 0;
  for (p = 0; src[p] != stop; ++p)
    dst[x++] = src[p];
  dst[x] = '\0';
}

static BaconDeviceList *
bacon_parse_local_for_device_list (const char *data)
{
  size_t l;
  size_t pos;
  char line[BACON_LINE_MAX];
  BaconDeviceList *p;
  BaconDeviceList *list;

  pos = 0;
  list = NULL;
  bacon_set_size_values ();

  while (BACON_TRUE) {
    bacon_get_line (line, data, &pos);
    l = 0;
    if (!*line)
      break;
    bacon_list_append (BaconDeviceList, list, p);
    p->device = bacon_new (BaconDevice);
    bacon_fill_buffer_pos (p->device->codename, line, &l, '@');
    bacon_fill_buffer_pos (p->device->fullname, line, &l, '\0');
    bacon_list_rewind (list, p);
  }
  return list;
}

static BaconDeviceList *
bacon_parse_remote_for_device_list (const char *data)
{
  char *d;
  char *x;
  BaconDeviceList *p;
  BaconDeviceList *list;

  d = NULL;
  list = NULL;
  bacon_set_size_values ();

  while (BACON_TRUE) {
    x = strstr ((!d) ? data : d, BACON_CODENAME_TAG);
    if (x && *x) {
      x = x + s_n_codename_tag;
      bacon_list_append (BaconDeviceList, list, p);
      p->device = bacon_new (BaconDevice);
      bacon_fill_buffer (p->device->codename, x, '<');
      d = x;
      bacon_find_and_fill (p->device->fullname, d, BACON_FULLNAME_TAG,
                           s_n_fullname_tag, x, '<');
      bacon_list_rewind (list, p);
    } else
      break;
  }
  return list;
}

BaconDeviceList *
bacon_parse_for_device_list (const char *data, BaconBoolean local)
{
  BaconDeviceList *list;

  if (local)
    list = bacon_parse_local_for_device_list (data);
  else
    list = bacon_parse_remote_for_device_list (data);
  return list;
}

BaconRom *
bacon_parse_for_rom (const char *data, int max)
{
  int m;
  char *x;
  char *d;
  BaconRom *p;
  BaconRom *rom;

  m = 0;
  d = NULL;
  rom = NULL;
  bacon_set_size_values ();
  
  while (m < max) {
    x = strstr ((!d) ? data : d, BACON_ROM_NAME_PATTERN);
    if (x && *x) {
      m++;
      x = x + s_n_rom_name_pattern;
      bacon_list_append (BaconRom, rom, p);
      bacon_fill_buffer (p->name, x, '<');
      d = x;
      bacon_find_and_fill (p->hash.hash, d, BACON_HASH_PATTERN,
                           s_n_hash_pattern, x, ' ');
      bacon_find_and_fill (p->get, d, BACON_GET_PATTERN,
                           s_n_get_pattern, x, '"');
      bacon_find_and_fill (p->size, d, BACON_SIZE_TAG, s_n_size_tag, x, '<');
      bacon_find_and_fill (p->date, d, BACON_DATE_TAG, s_n_date_tag, x, '<');
      bacon_list_rewind (rom, p);
    } else
      break;
  }
  return rom;
}

#ifdef BACON_GTK
BaconDeviceThumbRequestList *
bacon_parse_for_device_thumb_request_list (const char *data,
                                           BaconDeviceList *devicelist)
{
  char *d;
  char *e;
  char *x;
  BaconDeviceList *dp;
  BaconDeviceThumbRequestList *p;
  BaconDeviceThumbRequestList *list;

  e = NULL;
  x = NULL;
  list = NULL;
  bacon_set_size_values ();

  for (dp = devicelist; dp; dp = dp->next) {
    x = strstr (data, dp->device->codename);
    if (x && *x) {
      x = x + strlen (dp->device->codename);
      d = x;
      x = strstr (d, BACON_THUMB_URL_PATTERN);
      if (x && *x) {
        bacon_list_append (BaconDeviceThumbRequestList, list, p);
        x = x + s_n_thumb_url_pattern;
        bacon_fill_buffer (p->request, x, '"');
        if (p->request && *p->request) {
          e = strrchr (p->request, '.');
          if (!e || !*e)
            bacon_debug ("BUG: failed get thumb icon file extension "
                         "for '%s' - guessing '.png'",
                         dp->device->codename);
        } else
          bacon_debug ("BUG: failed to get thumb icon request URL for '%s'",
                       dp->device->codename);
        snprintf (p->filename, BACON_PATH_MAX, "device-%s%s",
                  dp->device->codename, (e && *e) ? e : ".png");
        bacon_list_rewind (list, p);
      }
    }
  }
  return list;
}
#endif

