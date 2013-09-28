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

#include <string.h>

#include "bacon.h"
#include "bacon-parse.h"
#include "bacon-util.h"

#define BACON_CODENAME_TAG     "<span class=\"codename\">"
#define BACON_FULLNAME_TAG     "<span class=\"fullname\">"
#define BACON_ROM_NAME_PATTERN ".zip\">"
#define BACON_HASH_PATTERN     "md5sum: "
#define BACON_GET_PATTERN      BACON_ROOT_URL "/"
#define BACON_SIZE_TAG         "<td>"
#define BACON_DATE_TAG         "<td>"

#define BACON_LINE_MAX 1024

#define bacon_append_new(type, root, p)   \
  do                                      \
  {                                       \
    if (!root)                            \
    {                                     \
      p = bacon_new (type);               \
      p->prev = NULL;                     \
    }                                     \
    else                                  \
    {                                     \
      for (p = root; p; p = p->next)      \
         if (!p->next)                    \
            break;                        \
      p->next = bacon_new (type);         \
      p->next->prev = p;                  \
      p = p->next;                        \
    }                                     \
    p->next = NULL;                       \
  } while (false)

#define bacon_back_to_start(root, p) \
  do                                 \
  {                                  \
    for (; p; p = p->prev)           \
      if (!p->prev)                  \
         break;                      \
    root = p;                        \
  } while (false)

#define bacon_find_and_fill(__dst, __src, __p, __np, __x, __c) \
  do                                                           \
  {                                                            \
    __x = strstr (__src, __p);                                 \
    if (__x && *__x)                                           \
    {                                                          \
      __x = __x + __np;                                        \
      bacon_fill_buffer (__dst, __x, __c);                     \
      __src = __x;                                             \
    }                                                          \
  } while (false)

static size_t n_codename_tag     = 0;
static size_t n_fullname_tag     = 0;
static size_t n_rom_name_pattern = 0;
static size_t n_hash_pattern     = 0;
static size_t n_get_pattern      = 0;
static size_t n_size_tag         = 0;
static size_t n_date_tag         = 0;

static void
bacon_set_size_values (void)
{
  if (!n_codename_tag)
    n_codename_tag = strlen (BACON_CODENAME_TAG);

  if (!n_fullname_tag)
    n_fullname_tag = strlen (BACON_FULLNAME_TAG);

  if (!n_rom_name_pattern)
    n_rom_name_pattern = strlen (BACON_ROM_NAME_PATTERN);

  if (!n_hash_pattern)
    n_hash_pattern = strlen (BACON_HASH_PATTERN);

  if (!n_get_pattern)
    n_get_pattern = strlen (BACON_GET_PATTERN);

  if (!n_size_tag)
    n_size_tag = strlen (BACON_SIZE_TAG);

  if (!n_date_tag)
    n_date_tag = strlen (BACON_DATE_TAG);
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
  size_t x;
  size_t l;
  size_t pos;
  char line[BACON_LINE_MAX];
  BaconDeviceList *p;
  BaconDeviceList *list;

  pos = 0;
  list = NULL;
  bacon_set_size_values ();

  while (true)
  {
    bacon_get_line (line, data, &pos);
    l = 0;
    if (!*line)
      break;
    bacon_append_new (BaconDeviceList, list, p);
    p->device = bacon_new (BaconDevice);
    bacon_fill_buffer_pos (p->device->codename, line, &l, '@');
    bacon_fill_buffer_pos (p->device->fullname, line, &l, '\0');
    bacon_back_to_start (list, p);
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

  while (true)
  {
    x = strstr ((!d) ? data : d, BACON_CODENAME_TAG);
    if (x && *x)
    {
      x = x + n_codename_tag;
      bacon_append_new (BaconDeviceList, list, p);
      p->device = bacon_new (BaconDevice);
      bacon_fill_buffer (p->device->codename, x, '<');
      d = x;
      bacon_find_and_fill (p->device->fullname, d, BACON_FULLNAME_TAG,
                           n_fullname_tag, x, '<');
      bacon_back_to_start (list, p);
    }
    else
      break;
  }
  return list;
}

BaconDeviceList *
bacon_parse_for_device_list (const char *data, bool local)
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
  
  while (m < max)
  {
    x = strstr ((!d) ? data : d, BACON_ROM_NAME_PATTERN);
    if (x && *x)
    {
      m++;
      x = x + n_rom_name_pattern;
      bacon_append_new (BaconRom, rom, p);
      bacon_fill_buffer (p->name, x, '<');
      d = x;
      bacon_find_and_fill (p->hash.hash, d, BACON_HASH_PATTERN,
                           n_hash_pattern, x, ' ');
      bacon_find_and_fill (p->get, d, BACON_GET_PATTERN,
                           n_get_pattern, x, '"');
      bacon_find_and_fill (p->size, d, BACON_SIZE_TAG, n_size_tag, x, '<');
      bacon_find_and_fill (p->date, d, BACON_DATE_TAG, n_date_tag, x, '<');
      bacon_back_to_start (rom, p);
    }
    else
      break;
  }
  return rom;
}

