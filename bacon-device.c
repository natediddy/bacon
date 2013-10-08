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

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bacon-device.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-util.h"

#define BACON_DEVICE_LIST_LOCAL_FILENAME "devicelist.txt"

extern char *g_program_data_path;
static char *s_local_device_list_path = NULL;

static void
bacon_set_local_device_list_path (void)
{
  s_local_device_list_path = bacon_strf ("%s%c%s",
                                         g_program_data_path,
                                         BACON_PATH_SEP,
                                         BACON_DEVICE_LIST_LOCAL_FILENAME);
}

static bool
bacon_has_local_device_list (void)
{
  return bacon_env_is_file (s_local_device_list_path);
}

static void
bacon_write_local_device_list (BaconDeviceList *list)
{
  FILE *fp;
  BaconDeviceList *p;

  fp = bacon_env_fopen (s_local_device_list_path, "w");
  for (p = list; p; p = p->next) {
    bacon_foutln (fp, "%s@%s", p->device->codename, p->device->fullname);
    if (!p->next)
      break;
  }
  bacon_env_fclose (fp);
}

static char *
bacon_device_local_data (void)
{
  int c;
  size_t n;
  size_t x;
  char *data;
  FILE *fp;
  struct stat s;

  n = 0;
  memset (&s, 0, sizeof (struct stat));
  if (stat (s_local_device_list_path, &s) == 0)
    n = ((size_t) s.st_size);

  data = bacon_newa (char, n);
  fp = bacon_env_fopen (s_local_device_list_path, "r");

  x = 0;
  while (true) {
    c = fgetc (fp);
    if (c == EOF)
      break;
    data[x++] = ((char) c);
  }

  data[x] = '\0';
  bacon_env_fclose (fp);
  return data;
}

BaconDeviceList *
bacon_device_list_new (const bool force_new)
{
  char *data;
  BaconDeviceList *list;

  data = NULL;
  bacon_set_local_device_list_path ();

  if (force_new || !bacon_has_local_device_list ()) {
    if (bacon_net_init_for_page_data ("")) {
      data = bacon_net_get_page_data ();
      if (data) {
        list = bacon_parse_for_device_list (data, false);
        bacon_write_local_device_list (list);
      }
      bacon_net_deinit ();
    }
  } else {
    data = bacon_device_local_data ();
    if (data)
      list = bacon_parse_for_device_list (data, true);
  }
  return list;
}

void
bacon_device_list_destroy (BaconDeviceList *list)
{
  for (; list; list = list->next) {
    bacon_free (list->device);
    bacon_free (list->prev);
    if (!list->next)
      break;
  }
  bacon_free (list);
}

int
bacon_device_list_total (BaconDeviceList *list)
{
  int total;
  BaconDeviceList *p;

  total = 0;
  for (p = list; p; p = p->next) {
    total++;
    if (!p->next)
      break;
  }
  return total;
}

bool
bacon_device_is_valid_id (BaconDeviceList *list, const char *id)
{
  BaconDeviceList *p;

  for (p = list; p; p = p->next) {
    if (bacon_streq (id, p->device->codename))
      return true;
    if (!p->next)
      break;
  }
  return false;
}

BaconDevice *
bacon_device_get_device_from_id (BaconDeviceList *list, const char *id)
{
  BaconDevice *device;
  BaconDeviceList *p;

  device = NULL;
  for (p = list; p; p = p->next) {
    if (bacon_streq (id, p->device->codename)) {
      device = p->device;
      break;
    }
    if (!p->next)
      break;
  }
  return device;
}

